<template>
  <div>
    <GmapMap style="width: 600px; height: 300px;" :zoom="10" :center="marker">
      <GmapMarker
        label="★"
        :position="marker"
        />
    </GmapMap>
  </div>
</template>

<script>
import api from '../store/api.js'
export default {
  data() {
    return {
      marker : {lat:null,lng:null},
    }
  },
  mounted: function(){
      setInterval(()=>{
        this.setPlace()
      },5000);
  },
  methods: {
    setPlace(place) {
      let self = this
       api.getdataByChannel(2).then(res=>{
          self.marker.lat = res.data.dataChannels[0].dataPoints[0].values.latitude
          self.marker.lng = res.data.dataChannels[0].dataPoints[0].values.longitude
      })
    },
  },
  beforeMount(){
      let self = this
      api.getdataByChannel(2).then(res=>{
          self.marker.lat = res.data.dataChannels[0].dataPoints[0].values.latitude
          self.marker.lng = res.data.dataChannels[0].dataPoints[0].values.longitude
      })
  }
}
</script>