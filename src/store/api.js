import axios from 'axios'

let client = axios.create({
     baseURL:   'https://api.mediatek.com'
    // timeout: 60000
})

let deviceID = "DMWq5R7i";
let deviceKey = "2XlQ1UH0qUW3CIJn";

export default {
    getdataByChannel : function(channel){
        let config = {
            headers : {
                'deviceKey': deviceKey ,
                'Content-type' : 'application/json' 
            }
        }
        return client.get("/mcs/v2/devices/" + deviceID + "/datachannels/" + channel + "/datapoints",config);
    },
    stepReset : function(data){
        console.log(JSON.stringify(data))
        let config = {
            headers: { 'deviceKey': deviceKey ,
            'Content-type': 'application/json',
            }
        }
        return client.post("/mcs/v2/devices/" + deviceID + "/datapoints",JSON.stringify(data),config);
    }
}