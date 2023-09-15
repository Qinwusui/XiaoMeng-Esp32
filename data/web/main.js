const { createApp, ref, reactive } = Vue

var wifiForm = ref({
    ssid: '',
    pwd: '',
})
const message = ref('Hello vue!')


const app = createApp({
    data() {
        return {
            message: message,
            wifiForm: wifiForm
        }
    },
    methods: {
        onSubmit() {
            axios.post('/wifi/submit', wifiForm.value)
        },
        onClear() {
            wifiForm.value = {}
        }
    }

})
app.use(ElementPlus)

app.mount('#app')