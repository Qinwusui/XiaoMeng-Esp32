
const { createApp, ref, reactive } = Vue
const { ElNotification } = ElementPlus
var wifiForm = ref({
    ssid: '',
    pwd: '',
})
const loadingWiFiList = ref(false)
var wifiList = ref([])
var currentWiFi = ref({})

var showWiFiConnectDialog = ref(false)
const wantConnectWiFiInfo = ref({})
const app = createApp({
    data() {
        return {
            wifiForm: wifiForm,
            loadingWiFiList: loadingWiFiList,
            wifiList: wifiList,
            currentWiFi: currentWiFi,
            showWiFiConnectDialog: showWiFiConnectDialog,
            wantConnectWiFiInfo: wantConnectWiFiInfo,
        }
    },
    methods: {
        lockBike() {
            axios.post('/bike/lock', {
                pass: 'Qinsansui233...',
                needLock: true
            })
        },
        unlockBike() {
            axios.post('/bike/lock', {
                pass: 'Qinsansui233...',
                needLock: false
            })
        },
        toBatteryManagerPage() {
            ElNotification({
                title: '提示',
                message: '这个页面还木有准备好哦',
                duration: 5
            })
            // window.location.href = '/battery/manager'
        },
        toWiFiPage() {
            window.location.href = '/wifi'
        },
        onSubmit(wifi) {
            showWiFiConnectDialog = false
            axios.post('/wifi/submit', wifi.value, {
                headers: {
                    'Content-Type': 'application/json'
                }
            }).then((e) => {
                let code = e["code"]
                let msg = e["msg"]
                if (code == 0) {
                    ElNotification({
                        title: '提示',
                        message: msg,
                        duration: 5
                    })
                } else {
                    ElNotification({
                        title: '提示',
                        message: msg,
                        duration: 5
                    })
                }
            }).catch((e) => {
                ElNotification({
                    title: '提示',
                    message: e,
                    duration: 5
                })
            })
        },
        onClear(wifi) {
            wifi.value = {}
        },
        toBikeManagerPage() {
            window.location.href = '/bike/manager'
        },
        toBatteryManagerPage() {
            ElNotification({
                title: '提示',
                message: '这个页面还木有准备好哦',
                duration: 5
            })
            // window.location.href = '/battery/manager'
        },
        runWiFiScanner() {
            loadingWiFiList.value = true
            axios.get('/startScan').then((res) => {
                loadingWiFiList.value = false
                console.log("执行到这")

                console.log(JSON.parse(res))
                wifiList.value = JSON.parse(res)
                ElNotification({
                    title: '提示',
                    message: JSON.parse(res),
                    duration: 2000
                })
            })
        },
        selectSingle(wifi) {
            currentWiFi.value = wifi
        },
        inputWiFiInfo() {
            if (currentWiFi.value.ssid == undefined) {
                ElNotification({
                    title: '提示',
                    message: '你还没有选择WiFi',
                    duration: 2000
                })
            } else {
                showWiFiConnectDialog.value = true
            }
        },

    }

})
app.use(ElementPlus)

app.mount('#app')