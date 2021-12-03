#![allow(missing_debug_implementations)]

use scylla::{transport::Compression, Bytes, QueryResult, Session, SessionBuilder};
use std::time::Duration;

pub struct ScyllaDBConnector {
    uri_known_nodes: Vec<String>,
    session: Option<Session>,
}

impl Default for ScyllaDBConnector {
    fn default() -> Self {
        Self::new()
    }
}

impl ScyllaDBConnector {
    pub fn new() -> Self {
        Self {
            uri_known_nodes: Vec::new(),
            session: None,
        }
    }

    pub async fn create_session(
        &mut self,
        p_know_nodes: Vec<String>,
        p_connection_timeout: Duration,
        p_username: &str,
        p_password: &str,
    ) -> anyhow::Result<()> {
        self.uri_known_nodes = p_know_nodes;

        if self.uri_known_nodes.is_empty() {
            anyhow::bail!("missing uri_known_nodes")
        }

        let mut session_builder = SessionBuilder::new();
        for uri in &self.uri_known_nodes {
            session_builder = session_builder.known_node(&uri);
        }
        self.session = Some(
            session_builder
                .connection_timeout(p_connection_timeout)
                .compression(Some(Compression::Lz4))
                .user(p_username, p_password)
                .build()
                .await?,
        );
        Ok(())
    }

    pub async fn query(
        &self,
        p_query: &str,
        p_values: impl scylla::frame::value::ValueList,
        p_paging_state: Option<Bytes>,
    ) -> anyhow::Result<QueryResult> {
        let ret = match &self.session {
            Some(s) => {
                let ret = s
                    .query_paged(p_query, p_values, p_paging_state)
                    .await
                    .or_else(|e| anyhow::bail!("query {} returns error {:?}", p_query, e));
                ret
            }
            None => anyhow::bail!("scylladb session was not provided"),
        };
        ret
    }
}

#[tokio::main]
#[test]
async fn test() {
    let mut scylla_db = ScyllaDBConnector::new();
    let ret = scylla_db
        .create_session(
            ["127.0.0.1:9042".to_string()].to_vec(),
            Duration::from_secs(3),
            "",
            "",
        )
        .await;

    println!("{:?}", ret);
}
