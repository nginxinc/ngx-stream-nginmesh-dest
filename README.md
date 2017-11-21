# NGINX Destination IP recovery module for stream

This dynamic module recovers original IP address and port number of the destination packet.
It is used by nginmesh sidecar where all outgoing traffic is redirect to a single port using iptable mechanism

## Dependencies

This module uses Linux **getsockopt** socket API.  
The installation uses Docker to build the module binary.

## Compatibility

* 1.11.x (last tested with 1.13.5)


## Synopsis

```nginx

 stream   {
 
	 server {
	 
			#  use iptable to capture all outgoing traffic.  see Istio design document
			listen 15001;
			
			# turn on module for this server
			# original IP destination and port is set to variable $nginmesh_dest
			# ex: 10.31.242.228:80
			nginmesh_dest on;
	
			# variable can be used in valid config directive
			proxy_pass $nginmesh_dest;
		}
		
 }	

```

## Embedded Variables

The following embedded variables are provided:

* **nginmesh_dest**
  * Original IP address and port in the format:  <address>:<port>

## Directives

### nginmesh_dest

| -   | - |
| --- | --- |
| **Syntax**  | **nginmesh_dest** \<on\|off\> |
| **Default** | off |
| **Context** | stream, server |

`Description:` Enables or disables the nginmesh_dest module


## Installation

1. Clone the git repository

  ```
  shell> git clone git@github.com:nginmesh/ngx-stream-nginmesh-dest.git
  ```

2. Build the dynamic module

  ```
  shell> make build-base;make build-module
  ```

  This copies the generated .so file into module/release directory



## Integration test

This works only on mac.

```bash
make test-nginx-only
make test-tcp
```