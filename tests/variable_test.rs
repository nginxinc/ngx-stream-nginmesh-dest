extern crate futures;
extern crate hyper;
extern crate tokio_core;


#[cfg(test)]
mod tests {

    use futures::Future;
    use hyper::Client;
    use hyper::{Method, Request};
    use hyper::StatusCode;
    use tokio_core::reactor::Core;


    #[test]
    fn test() {

        let mut core = Core::new().unwrap();

        let client = Client::new(&core.handle());

        // curl -H "Host: u1" localhost
        let uri = "http://localhost".parse().unwrap();
        let mut req = Request::new(Method::Get, uri);
        req.headers_mut().set_raw("Host", "u1");
        let work = client.request(req).map(|res| {
            let status = res.status();
            println!("Response: {}", status);
            assert_eq!(status, StatusCode::Ok);

        });

        core.run(work).unwrap();
    }


}
