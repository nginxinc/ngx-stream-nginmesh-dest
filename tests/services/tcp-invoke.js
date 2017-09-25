const net = require('net');


const port = process.argv[2] || 8000;

console.log('listening to port',port);

const DestIp = '8.8.8.8';

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

    function onConnData(d) {
        console.log('connection data from %s: %j', remoteAddress, d);
        conn.write("hello");
        connectToDest(DestIp);
    }

    function onConnClose() {
        console.log('connection from %s closed', remoteAddress);
    }

    function onConnError(err) {
        console.log('Connection %s error: %s', remoteAddress, err.message);
    }
}



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

