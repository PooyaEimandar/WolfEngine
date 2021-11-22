use anyhow::{anyhow, bail, Result};
use std::path::Path;
use tonic::transport::{Certificate, Channel, ClientTlsConfig, Endpoint};

pub async fn run_server<S>(
    p_address: &str,
    p_port: u16,
    p_tls: bool,
    p_tls_certificate_path: Option<&Path>,
    p_tls_private_key_path: Option<&Path>,
    p_service: S,
) -> anyhow::Result<()>
where
    S: tonic::codegen::Service<
            hyper::Request<hyper::Body>,
            Response = hyper::Response<tonic::body::BoxBody>,
        > + tonic::transport::NamedService
        + Clone
        + Send
        + 'static,
    S::Future: Send + 'static,
    S::Error: Into<Box<dyn std::error::Error + Send + Sync>> + Send,
{
    const TRACE: &str = "run_server";
    use tonic::transport::Identity;

    let addr = format!("{}:{}", p_address, p_port).parse()?;
    if p_tls {
        let p = Path::new("");
        //load certificate and private key for tls
        let cert_path = p_tls_certificate_path.unwrap_or(p);
        let key_path = p_tls_private_key_path.unwrap_or(p);

        if !cert_path.exists() {
            anyhow::bail!(
                "path to the tls certificate was not provided. trace {}",
                TRACE
            )
        }
        if !key_path.exists() {
            bail!(
                "path to the tls private key was not provided. trace {}",
                TRACE
            )
        }

        //load certificates
        let cert = tokio::fs::read(cert_path).await?;
        let key = tokio::fs::read(key_path).await?;
        let identity = Identity::from_pem(cert, key);
        let server_tls_config = ServerTlsConfig::new().identity(identity);

        use tonic::transport::ServerTlsConfig;
        let ret = tonic::transport::Server::builder()
            .tls_config(server_tls_config)?
            .add_service(p_service)
            .serve(addr)
            .await
            .map_err(|e| {
                anyhow!("could not build grpc server because {:?}", e).context("grpc::run_server")
            });
        ret
    } else {
        let ret = tonic::transport::Server::builder()
            .add_service(p_service)
            .serve(addr)
            .await
            .map_err(|e| {
                anyhow!("could not build grpc server because {:?}", e).context("grpc::run_server")
            });
        ret
    }
}

///create a tls grpc endpoint
pub async fn create_tls_endpoint(
    p_end_point: String,
    p_domain_name: String,
    p_tls_certificate: &Path,
) -> Result<Endpoint> {
    const TRACE: &str = "create_grpc_channel";
    let pem_res = tokio::fs::read(p_tls_certificate).await;
    let res = match pem_res {
        Ok(pem) => {
            //create endpoint
            let endpoint_res = Channel::from_shared(p_end_point.clone());
            let res = match endpoint_res {
                Ok(endpoint) => {
                    //load tls certificate
                    let cert = Certificate::from_pem(pem);
                    //create tls config for domain
                    let tls = ClientTlsConfig::new()
                        .ca_certificate(cert)
                        .domain_name(p_domain_name);

                    //configures TLS for the endpoint
                    let ret = endpoint
                        .tls_config(tls)
                        .and_then(|end| Ok(end))
                        .or_else(|e| {
                            bail!(
                                "could not create grpc tls_config for endpoint {} because: {:?}. trace : {}",
                                p_end_point,
                                e,
                                TRACE
                            )
                        });
                    ret
                }
                Err(e) => bail!(
                    "grpc endpoint uri {} is invalid because: {:?}. trace: {}",
                    p_end_point,
                    e,
                    TRACE
                ),
            };
            res
        }
        Err(e) => bail!(
            "could not load certificate for grpc client because: {:?}. trace: {}",
            e,
            TRACE
        ),
    };
    res
}

///create a tls channel for connecting to grpc server
pub async fn create_tls_channel(p_tls_endpoint: &Endpoint) -> Result<Channel> {
    const TRACE: &str = "create_tls_channel";
    //configures TLS for the endpoint
    let ret = p_tls_endpoint
        .connect()
        .await
        .and_then(|channel| {
            //return channel
            anyhow::Result::Ok(channel)
        })
        .or_else(|e| {
            bail!(
                "could not create channel for grpc on endpoint {} because: {:?}. trace: {}",
                p_tls_endpoint.uri().to_string(),
                e,
                TRACE
            )
        });
    ret
}

///create a none tls channel for connecting to grpc server
pub async fn create_channel(p_end_point: String) -> Result<Channel> {
    const TRACE: &str = "create_channel";
    //create endpoint
    let endpoint_res = Channel::from_shared(p_end_point.clone());
    let res = match endpoint_res {
        Ok(endpoint) => {
            endpoint
                .connect()
                .await
                .and_then(|channel| {
                    //return channel
                    anyhow::Result::Ok(channel)
                })
                .or_else(|e| {
                    bail!(
                        "could not create channel for grpc client on endpoint {} because: {:?}. trace: {}",
                        p_end_point,
                        e,
                        TRACE
                    )
                })
        }
        Err(e) => bail!(
            "endpoint uri {} is invalid because: {:?}. trace: {}",
            p_end_point,
            e,
            TRACE
        ),
    };
    res
}
