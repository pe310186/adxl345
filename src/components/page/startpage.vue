<template>
    <v-container>
        <center>
            
            <br>
            <p>電池電量: {{battery}} &nbsp;&nbsp;&nbsp;</p>
            <br>
            <p>地圖資訊:</p>
            <maps></maps>
        </center>
    </v-container>
</template>

<script>

import api from '../../store/api.js'
export default {
    data(){
        return {
            step:'',
            battery:'',
        }
    },
    mounted:function(){
        setInterval(()=>{
        this.getdata()
      },1000);
    },
    methods:{
        stepReset(){
            let self = this
            var data = {
            datapoints:
                [{dataChnId: "1", values: {value: 0}}]
            }
            api.stepReset(data).then(res=>{
                alert('重置成功')
                api.getdataByChannel(1).then(res=>{
                    self.step = res.data.dataChannels[0].dataPoints[0].values.value
                }).catch(error=>{

                })
            })
        },
        getdata(){
            let self = this
            //step
            api.getdataByChannel(1).then(res=>{
                self.step = res.data.dataChannels[0].dataPoints[0].values.value
            }).catch(error=>{

            })

            
            //battery
            api.getdataByChannel(3).then(res=>{
                self.battery = res.data.dataChannels[0].dataPoints[0].values.value
            }).catch(error=>{

            })
            
        }
    },
    beforeMount(){
        let self = this
        //step
        api.getdataByChannel(1).then(res=>{
            self.step = res.data.dataChannels[0].dataPoints[0].values.value
        }).catch(error=>{

        })

        
        //battery
        api.getdataByChannel(3).then(res=>{
            self.battery = res.data.dataChannels[0].dataPoints[0].values.value
        }).catch(error=>{

        })
    }
}
</script>

<style>

</style>
