# Nginx Nginmesh Stream Dest IP Module

This repo implements NGINX modules getting destination IP and port number.
It is used in context where all outgoing traffic is funnel thru a single port.
So we need original destination IP and port to route to proper upstream.
This is used in the Istio proxy configuration.

## Usage

<TBD>

## Check out Nginx Rust Module

```bash
giut clone git@github.com:nginmesh/ngx-rust.git
```

Rust module needs to be check out at same level as this project.
Follow instruction in Rust module and configure for each of the target OS.



## Configure and Build

Before building, it must be configured for each of the target OS.

### For Linux

To configure:

```bash
make linux-setup
```

Generating module:


```bash
make linux-module
```

Generated module is founded at:

```bash
ls nginx/nginx-linux/objs/ngx_ngin_mesh_module.so
```


### For Mac

To configure:

```bash
make ngin-setup
```

Generating module:


```bash
make ngin-module
```


### Run Integration test using Docker

```bash
make linux-test-start
```

This will create test docker container to start nginx as if it is running as sidecar

```bash
make linux-test-nc
```

This will trigger tcp server in the test container