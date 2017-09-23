const http = require('http');


const port = process.argv[2] || 8000;

console.log('listening to port',port);

const ipServiceUrl = "http://api.ipify.org";

http.createServer( async (request, response) => {

    const reply = await ipService(ipServiceUrl);
    console.log("received: "+reply);
    response.writeHead(200, {'Content-type':'text/plan'});
    response.write(`${reply}`);
    response.end( );
}).listen(port);


function ipService(url)  {

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