
const { createApp, ref, reactive } = Vue
var wifiForm = ref({
    ssid: '',
    pwd: '',
})
const app = createApp({
    data() {
        return {
            wifiForm: wifiForm
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
        onSubmit() {
            axios.post('/wifi/submit', wifiForm.value, {
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
        onClear() {
            wifiForm.value = {}
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
        }
    }

})
app.use(ElementPlus)

app.mount('#app')