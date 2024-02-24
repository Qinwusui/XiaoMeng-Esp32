
const { createApp, ref, reactive } = Vue
const { ElNotification, ElTable } = ElementPlus
var wifiForm = ref({
    ssid: '',
    pwd: '',
})
var weatherForm = ref(
    {
        location: '',
        key: ''
    }
)
const loadingWiFiList = ref(false)

const wifiList = ref([])


const singleTableRef = ref(ElTable)

const showWiFiConnectDialog = ref(false)
const showWeatherConfigDialog = ref(false)
const currentWiFiStatus = ref('')
const wantConnectWiFiInfo = ref({})
const app = createApp({
    data() {
        return {
            wifiForm: wifiForm,
            weatherForm: weatherForm,
            loadingWiFiList: loadingWiFiList,
            wifiList: wifiList,
            showWiFiConnectDialog: showWiFiConnectDialog,
            showWeatherConfigDialog: showWeatherConfigDialog,
            wantConnectWiFiInfo: wantConnectWiFiInfo,
            singleTableRef: singleTableRef,
            currentWiFiStatus: currentWiFiStatus
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
        onSubmitWiFiConfig() {
            showWiFiConnectDialog.value = false
            console.log(wifiForm.value)
            if (wifiForm.value == {}) {
                console.log("WiFi信息为空")
                return
            }
            axios.post('/wifi/submit', wifiForm.value, {
                headers: {
                    'Content-Type': 'application/json'
                }
            }).then((e) => {
                let code = e.code
                let msg = e.msg
                ElNotification({
                    title: '提示',
                    message: msg,
                    duration: 2000
                })
            }).catch((e) => {
                ElNotification({
                    title: '提示',
                    message: e,
                    duration: 2000
                })
            })
        },
        onClearWiFiConfig() {
            wifiForm.value = {}
        },
        onSubmitWeatherConfig() {
            showWeatherConfigDialog.value = false
            if (weatherForm.value == {}) {
                console.log("天气配置信息为空")
                return
            }
            axios.post('/weather/submit', weatherForm.value, {
                headers: {
                    'Content-Type': 'application/json'
                }
            }).then((e) => {
                let msg = e.msg
                ElNotification({
                    title: '提示',
                    message: msg,
                    duration: 2000
                })
            }).catch((e) => {
                ElNotification({
                    title: '提示',
                    message: e,
                    duration: 2000
                })
            })
        },
        onClearWeatherConfig() {
            weatherForm.value = {}
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
        async runWiFiScanner() {
            loadingWiFiList.value = true
            await axios.get('/wifi/startScan').then((res) => {
                loadingWiFiList.value = false
                console.log("执行到这")
                let data = res.data

                wifiList.value = data.list
                ElNotification({
                    title: '提示',
                    message: '扫描完成',
                    duration: 2000
                })
            })
        },
        selectSingle(wifi) {
            wifiForm.value = wifi
            console.log(wifiForm.value)
        },
        inputWiFiInfo() {
            if (wifiForm.value == {}) {
                ElNotification({
                    title: '提示',
                    message: '你还没有选择WiFi',
                    duration: 2000
                })
            } else {
                showWiFiConnectDialog.value = true
            }
        },
        inputWeatherInfo() {
            showWeatherConfigDialog.value = true
        },
        getCurrentWiFiStatus() {
            axios.get("/wifi/status").then((res) => {
                let data = res.data
                let status = data.status
                let statusStr = ''
                switch (status) {
                    case 0:
                        statusStr = '初始化'
                        break;
                    case 2:
                        statusStr = '扫描完成'
                        break
                    case 3:
                        statusStr = '已连接'
                        break
                    case 4:
                        statusStr = '连接失败'
                        break
                    case 5:
                        statusStr = '连接已丢失'
                        break
                    case 6:
                        statusStr = '已断开连接'

                    default:
                        statusStr = '未知状态'
                        break;
                }
                currentWiFiStatus.value = statusStr
                ElNotification({
                    title: '当前WiFi状态',
                    message: statusStr,
                    duration: 3000
                })
            })
        }

    }

})
app.use(ElementPlus)

app.mount('#app')