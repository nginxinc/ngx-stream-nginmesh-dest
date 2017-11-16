const net = require('net');
const http = require('http');

const port = process.argv[2] || 8000;
const destFlag = process.argv[3];

console.log('listening to port',port);

const DestIp = '8.8.8.8';
const localService = "http://localhost:5000";

const server = net.createServer();
server.on('connection', handleConnection);

server.listen(port, () => {
    console.log('server listening to %j', server.address());
});

function handleConnection(conn) {
    const remoteAddress = conn.remoteAddress + ':' + conn.remotePort;
    console.log('new client connection from %s', remoteAddress);

    conn.setEncoding('utf8');
    conn.on('data', onConnData);
    conn.once('close', onConnClose);
    conn.on('error', onConnError);

    async function onConnData(d) {
        console.log('connection data from %s: %j', remoteAddress, d);
        const result = await makeHttpCall(localService);
        console.log('received: local http service'+result);
        conn.write(result);
        if(destFlag === 'dest') {
            connectToDest(DestIp);
        }
    }

    function onConnClose() {
        console.log('connection from %s closed', remoteAddress);
    }

    function onConnError(err) {
        console.log('Connection %s error: %s', remoteAddress, err.message);
    }
}


// make http request
function connectToDest(destIp) {
    console.log(`trying to connect to ${destIp}`);


    const client = new net.Socket();
    client.connect(53, destIp, function() {
        console.log(`Connected to ${destIp}`);
        client.write('Sending....');
    });

    client.on('data', function(data) {
        console.log('Received: ' + data);
        client.destroy(); // kill client after server's response
    });

    client.on('close', function() {
        console.log(`connectio to ${destIp} closed`);
    });
}


// make http call
function makeHttpCall(url)  {

    return new Promise((resolve,reject) => {
        console.log('sending out to '+url);
        http.get(url, res => {
            res.setEncoding("utf8");
            let body = "";
            res.on("data", data => {
                console.log('received body:'+data);
                body += data;
            });
            res.on("end", () => {
                console.log('received end');
                resolve(body);
            });
            res.on("error",(err)=>{
                reject(err);
            });
        });
    });


}


