import Vue from 'vue'
import App from './App'
import Vuetify from 'vuetify'
import router from './router'
import Components from './components/_index'
import * as VueGoogleMaps from 'vue2-google-maps'

Vue.config.productionTip = false

Vue.use(Vuetify)

Vue.use(VueGoogleMaps, {
  load: {
    key: 'AIzaSyA6rhMEkKRMBQ6M8n4QsxUPv3cG7-V-w2w',
    libraries: 'places',
  },
})


Object.keys(Components).forEach(key => {
  Vue.component(key, Components[key])
})
new Vue({
  el: '#app',
  router,
  render: h => h(App)
})
