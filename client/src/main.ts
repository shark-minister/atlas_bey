import { createApp } from "vue"
import App from "./App.vue"
import './styles/common.scss'

import router from "./controls/router"

import "vuetify/styles"
import { createVuetify } from "vuetify"

import * as components from 'vuetify/components'
import * as directives from 'vuetify/directives'
import { aliases, mdi } from 'vuetify/iconsets/mdi-svg'

const vuetify = createVuetify({
  components,
  directives,
  // テーマ
  theme: {
    defaultTheme: "light"
  },
  icons: {
    aliases,
    sets: {
      mdi,
    }
  }
})

createApp(App)
  .use(router)
  .use(vuetify)
  .mount("#app")
