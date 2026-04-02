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
  history: createWebHistory(),
  routes: [
    { path: "/atlas_bey/", component: Home },
    { path: "/atlas_bey/statistics", component: Statistics },
    { path: "/atlas_bey/profiles", component: Profiles },
    { path: "/atlas_bey/parameters", component: Parameters }
  ]
})

router.beforeEach((to) => {
  if (to.path !== "/atlas_bey/" && !isConnected) {
    return "/atlas_bey/"
  }
})

export default router
