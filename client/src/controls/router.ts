import { createRouter, createWebHistory } from "vue-router"

import Home from "../pages/Home.vue"
import Statistics from "../pages/Statistics.vue"
import Profiles from "../pages/Profiles.vue"
import Parameters from "../pages/Parameters.vue"
import { useATLAS } from "@/classes/atlas"

const {
  isConnected,
} = useATLAS()

const router = createRouter({
  history: createWebHistory('/atlas_bey/'),
  routes: [
    { path: "/", component: Home },
    { path: "/statistics", component: Statistics },
    { path: "/profiles", component: Profiles },
    { path: "/parameters", component: Parameters }
  ]
})

router.beforeEach((to) => {
  const { isConnected } = useATLAS()

  if (to.path !== "/" && !isConnected.value) {
    return "/"
  }
})

export default router
