use crate::algorithm::raft::{
    raft_converter, raft_srv::wolf_raft, raft_srv::wolf_raft::raft_client::RaftClient,
};
use crate::net::grpc;
use anyhow::{bail, Result};
use async_raft::{
    raft::{self, InstallSnapshotRequest, InstallSnapshotResponse, VoteRequest, VoteResponse},
    Config, NodeId, RaftNetwork,
};
use async_trait::async_trait;
use memstore::{ClientRequest, ClientResponse, MemStore};
use std::sync::Arc;
use uuid::Uuid;

pub type MemRaft = async_raft::Raft<ClientRequest, ClientResponse, RaftRouter, MemStore>;
const BASE_PORT: u64 = 7777;

#[derive(Debug)]
pub struct RaftRouter {}

impl Default for RaftRouter {
    fn default() -> Self {
        Self::new()
    }
}

impl RaftRouter {
    /// Create a new RaftRouter instance.
    pub fn new() -> Self {
        Self {}
    }
}

#[async_trait]
impl RaftNetwork<ClientRequest> for RaftRouter {
    /// Send an AppendEntries RPC to the target Raft node
    async fn append_entries(
        &self,
        p_target_node: NodeId,
        p_rpc: raft::AppendEntriesRequest<ClientRequest>,
    ) -> Result<raft::AppendEntriesResponse> {
        const TRACE: &str = "raft_imp:append_entries";

        let uuid = Uuid::new_v5(&Uuid::NAMESPACE_X500, "wolf_raft_append_entries".as_bytes());
        let res = raft_converter::raft_append_entries_req_to_grpc_append_entries_req(
            uuid.to_string(),
            &p_rpc,
        );
        let ret = match res {
            Ok(req) => {
                //create a channel for grpc
                let uri = format!("http://localhost:{}", BASE_PORT + p_target_node);
                let res_1 = grpc::create_channel(uri).await;
                let ret_1 = match res_1 {
                    Ok(c) => {
                        //call request with channel
                        let mut client = RaftClient::new(c).send_gzip().accept_gzip();
                        let res_2 = client.append_entries(req).await;
                        let ret_2 = match res_2 {
                            Ok(r) => {
                                let ret_3 = match r.into_inner().response {
                                    Some(s) => {
                                        use wolf_raft::raft_append_entries_res::*;
                                        if let Response::OkRes(ok) = s {
                                            //create AppendEntriesResponse from RaftAppendEntriesOkRes
                                            let ret = raft_converter::grpc_append_entries_ok_res_to_raft_append_entries_res(&ok);
                                            Ok(ret)
                                        } else if let Response::ErrorRes(e) = s {
                                            bail!(
                                                "AppendEntriesResponse for node {} contains Error {:?}. Trace: {}",
                                                p_target_node,
                                                e,
                                                TRACE
                                            )
                                        } else {
                                            bail!(
                                                "AppendEntriesResponse for node {} contains Unknown error. Trace: {}",
                                                p_target_node,
                                                TRACE
                                            )
                                        }
                                    }
                                    None => {
                                        bail!(
                                            "inner message of AppendEntriesResponse is None for node {}. Trace: {}",
                                            p_target_node,
                                            TRACE
                                        )
                                    }
                                };
                                ret_3
                            }
                            Err(e) => {
                                bail!(
                                    "AppendEntriesResponse for node {} contains error status {:?}. Trace: {}",
                                    p_target_node,
                                    e,
                                    TRACE
                                )
                            }
                        };
                        ret_2
                    }
                    Err(e) => {
                        bail!(
                            "could not create a grpc channel while sending raft::AppendEntriesResponse for node {} because: {:?}. Trace: {}",
                            p_target_node,
                            e,
                            TRACE
                        )
                    }
                };
                ret_1
            }
            Err(e) => {
                bail!("{:?}. trace: {}", e, TRACE)
            }
        };
        ret
    }

    /// Send an InstallSnapshot RPC to the target Raft node
    async fn install_snapshot(
        &self,
        p_target_node: u64,
        p_rpc: InstallSnapshotRequest,
    ) -> Result<InstallSnapshotResponse> {
        const TRACE: &str = "raft_imp::install_snapshot";

        //create a channel for grpc
        let uri = format!("http://localhost:{}", BASE_PORT + p_target_node);
        let res = crate::net::grpc::create_channel(uri).await;
        let ret = match res {
            Ok(c) => {
                //call request with channel
                let uuid = Uuid::new_v5(
                    &Uuid::NAMESPACE_X500,
                    "wolf_raft_install_snapshot".as_bytes(),
                );
                let msg_id = uuid.to_string();
                let rpc_req =
                    raft_converter::raft_install_snapshot_req_to_grpc_install_snapshot_req(
                        msg_id, &p_rpc,
                    );

                let mut client = RaftClient::new(c).send_gzip().accept_gzip();
                let res_1 = client.install_snapshot(rpc_req).await;
                let ret_1 = match res_1 {
                    Ok(r) => {
                        let ret_2 = match r.into_inner().response {
                            Some(s) => {
                                use wolf_raft::raft_install_snapshot_res::*;
                                if let Response::OkRes(ok) = s {
                                    let response = raft_converter::grpc_install_snapshot_ok_res_to_raft_install_snapshot_res(&ok);
                                    Ok(response)
                                } else if let Response::ErrorRes(e) = s {
                                    bail!(
                                        "InstallSnapshotResponse for node {} contains Error {:?}. Trace: {}",
                                        p_target_node,
                                        e,
                                        TRACE
                                    )
                                } else {
                                    bail!(
                                        "InstallSnapshotResponse for node {} contains Unknown error. Trace: {}",
                                        p_target_node,
                                        TRACE
                                    )
                                }
                            }
                            None => {
                                bail!(
                                    "inner message of InstallSnapshotResponse is None for node {}. Trace: {}",
                                    p_target_node,
                                    TRACE
                                )
                            }
                        };
                        ret_2
                    }
                    Err(e) => {
                        bail!(
                            "InstallSnapshotResponse for node {} contains error status {:?}. Trace: {}",
                            p_target_node,
                            e,
                            TRACE
                        )
                    }
                };
                ret_1
            }
            Err(e) => {
                bail!(
                    "could not create grpc channel on sending raft::InstallSnapshotResponse for node {} because: {:?}. Trace: {}",
                    p_target_node,
                    e,
                    TRACE
                )
            }
        };
        ret
    }

    /// Send an Vote RPC to the target Raft node
    async fn vote(&self, p_target_node: u64, p_rpc: VoteRequest) -> Result<VoteResponse> {
        const TRACE: &str = "raft_imp::vote";

        //create a channel for grpc
        let uri = format!("http://localhost:{}", BASE_PORT + p_target_node);
        let res = crate::net::grpc::create_channel(uri).await;
        let ret = match res {
            Ok(c) => {
                //call request with channel
                let uuid = Uuid::new_v5(&Uuid::NAMESPACE_X500, "wolf_raft_vote".as_bytes());
                let msg_id = uuid.to_string();
                let rpc_req = raft_converter::raft_vote_req_to_grpc_vote_req(msg_id, &p_rpc);
                let mut client = RaftClient::new(c).send_gzip().accept_gzip();
                let res_1 = client.vote(rpc_req).await;
                let ret_1 = match res_1 {
                    Ok(r) => {
                        let ret_2 = match r.into_inner().response {
                            Some(s) => {
                                use wolf_raft::raft_vote_res::*;
                                if let Response::OkRes(ok) = s {
                                    let res =
                                        raft_converter::grpc_vote_ok_res_to_raft_vote_res(&ok);
                                    Ok(res)
                                } else if let Response::ErrorRes(e) = s {
                                    bail!(
                                        "VoteResponse for node {} contains Error {:?}. Trace: {}",
                                        p_target_node,
                                        e,
                                        TRACE
                                    )
                                } else {
                                    bail!(
                                        "VoteResponse for node {} contains Unknown error. Trace: {}",
                                        p_target_node,
                                        TRACE
                                    )
                                }
                            }
                            None => {
                                bail!(
                                    "inner message of VoteResponse is None for node {}. Trace: {}",
                                    p_target_node,
                                    TRACE
                                )
                            }
                        };
                        ret_2
                    }
                    Err(e) => {
                        bail!(
                            "VoteResponse for node {} contains error status {:?}. Trace: {}",
                            p_target_node,
                            e,
                            TRACE
                        )
                    }
                };
                ret_1
            }
            Err(e) => {
                bail!(
                    "could not create grpc channel on sending raft::VoteResponse for node {} because: {:?}. Trace: {}",
                    p_target_node,
                    e,
                    TRACE)
            }
        };
        ret
    }
}

/// create a MemRaft node with specific node id and cluster information
pub fn new(p_cluster_name: &str, p_node_id: u64) -> MemRaft {
    let config = Config::build(p_cluster_name.into())
        .validate()
        .unwrap_or_else(|e| {
            panic!(
                "failed to build Raft config for cluster:{} and node id:{}. error: {}",
                p_cluster_name, p_node_id, e
            )
        });
        
    //now create MemRaft node
    let arc_config = Arc::new(config);
    let network = Arc::new(RaftRouter::new());
    let storage = Arc::new(MemStore::new(p_node_id));
    raft::Raft::new(p_node_id, arc_config, network, storage)
}
