/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"

#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "camera_pins.h"

static const char *TAG = "eth_cam";
#define INDEX_HTML_PATH "/spiffs/index.html"
#define CONFIG_XCLK_FREQ 20000000 

#if REMARKED
/*
const char index_html[] = \
"<!DOCTYPE HTML><html>\r\n"\
"<head></head>\r\n"\
"<body>\r\n"\
"  <div><img src=\"caption\" id=\"photo\" width=\"70%\"></div>\r\n"\
"<script>"
"setInterval(function() {"
    "var ie = document.getElementById(\"photo\");"
    "ie.src = \"caption.jpg?rand=\" + Math.random();"
"}, 2000);"
"</script>"
"</body>\r\n"\
"</html>";
*/
/*
const char index_html_xxx[] =
"<!DOCTYPE HTML><html>"
"<head>"
"</head>"
<html>
<body>
<div id="video">
<img src='image1.jpg' width='1000' height='500' id="image"></img>
</div>
<script>
window.onload = refreshBlock;
function refreshBlock()
{
    document.getElementById("image").src="image4.jpg";
    setTimeout("refreshBlock",1000);
}
</script>
</body>
</html>
*/

"<head>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"<style>"
"* {box-sizing: border-box;}"
"body {font-family: Verdana, sans-serif;}"
".mySlides {display: none;}"
"img {vertical-align: middle;}"

".slideshow-container {"
  "max-width: 1000px;"
  "position: relative;"
  "margin: auto;"
"}"

".text {"
  "color: #f2f2f2;"
  "font-size: 15px;"
  "padding: 8px 12px;"
  "position: absolute;"
  "bottom: 8px;"
  "width: 100%;"
  "text-align: center;"
"}"

".numbertext {"
  "color: #f2f2f2;"
  "font-size: 12px;"
  "padding: 8px 12px;"
  "position: absolute;"
  "top: 0;"
"}"

"@media only screen and (max-width: 300px) {"
  ".text {font-size: 11px}"
"}"
"</style>"
"</head>"
"<body>"

"<h2>Automatic Slideshow</h2>"
"<p>Change image every 2 seconds:</p>"

"<div class=\"slideshow-container\">"

"<div class=\"mySlides\">"
  "<div class=\"numbertext\">1 / 3</div>"
  "<img src=\"caption.jpg\" style=\"width:100%\">"
  "<div class=\"text\">Caption Text</div>"
"</div>"

"</div>"
"<br>"

"<script>"
"let slideIndex = 0;"
"showSlides();"

"function showSlides() {"
  "let i;"
  "let slides = document.getElementsByClassName(\"mySlides\");"
  "for (i = 0; i < slides.length; i++) {"
    "slides[i].style.display = \"none\";"
  "}"
  "slideIndex++;"
  "if (slideIndex > slides.length) {slideIndex = 1}"
  "slides[slideIndex-1].style.display = \"block\";"
  "setTimeout(showSlides, 2000);" // Change image every 2 seconds
"}"
"</script>"

"</body>"
"</html>)";

"<head>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<style>"
    "body { text-align:center; }"
    ".vert { margin-bottom: 10%; }"
    ".hori{ margin-bottom: 0%; }"
  "</style>"
"</head>"
"<body>"
  "<div id=\"container\">"
    "<h2>ESP32-CAM Last Photo</h2>"
    "<p>It might take more than 5 seconds to capture a photo.</p>"
    "<p>"
      "<button onclick=\"rotatePhoto();\">ROTATE</button>"
      "<button onclick=\"capturePhoto()\">CAPTURE PHOTO</button>"
      "<button onclick=\"location.reload();\">REFRESH PAGE</button>"
    "</p>"
  "</div>"
  "<div><img src=\"saved-photo\" id=\"photo\" width=\"70%\"></div>"
"</body>"
"<script>"
  "var deg = 0;"
  "function capturePhoto() {"
    "var xhr = new XMLHttpRequest();"
    "xhr.open('GET', \"/capture\", true);"
    "xhr.send();"
  "}"
  "function rotatePhoto() {"
    "var img = document.getElementById(\"photo\");"
    "deg += 90;"
    "if(isOdd(deg/90)){ document.getElementById(\"container\").className = \"vert\"; }"
    "else{ document.getElementById(\"container\").className = \"hori\"; }"
    "img.style.transform = \"rotate(\" + deg + \"deg)\";"
  "}"
  "function isOdd(n) { return Math.abs(n % 2) == 1; }"
"</script>"
"</html>)";
#endif

char *g_index_html;

static esp_err_t init_camera(void){
    camera_config_t camera_config = {
        .pin_pwdn  = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = CONFIG_XCLK_FREQ,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_RGB565,
        //.pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_96X96, //= FRAMESIZE_QVGA, //FRAMESIZE_VGA,

        .jpeg_quality = 10, //0-63, for OV series camera sensors, lower number means higher quality
        .fb_count = 1,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY //CAMERA_GRAB_LATEST. Sets when buffers should be filled
    };

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        return err;
    }
    return ESP_OK;
}

#define TO_LITTLE_ENDOAN(c) (((uint16_t)c >> 8) | ((uint16_t)c << 8))

//rrrrrggg gggbbbbb
#define COL_BLUE  TO_LITTLE_ENDOAN(0x001f)
#define COL_GREEN TO_LITTLE_ENDOAN(0x07e0)
#define COL_RED   TO_LITTLE_ENDOAN(0xf800)

void putpixel_rgb565(camera_fb_t *fb, int x, int y, uint16_t c){
    //*(uint16_t*)&fb->buf[(y*fb->width + x)*2] = c;
    ((uint16_t*)fb->buf)[(y*fb->width + x)] = c;
}

void draw_circle_parts(camera_fb_t *fb, int xc, int yc, int x, int y, uint16_t c){
    putpixel_rgb565(fb, xc+x, yc+y, c);
	putpixel_rgb565(fb, xc-x, yc+y, c);
	putpixel_rgb565(fb, xc+x, yc-y, c);
	putpixel_rgb565(fb, xc-x, yc-y, c);
	putpixel_rgb565(fb, xc+y, yc+x, c);
	putpixel_rgb565(fb, xc-y, yc+x, c);
	putpixel_rgb565(fb, xc+y, yc-x, c);
	putpixel_rgb565(fb, xc-y, yc-x, c);
}

void draw_circle_midpoint(camera_fb_t *fb, int xc, int yc, int r, uint16_t c){
    int x = r;
    int y = 0;
    int err = 0;
    while (x >= y){
        draw_circle_parts(fb, xc, yc, x, y, c);
        if (err <= 0){
          y += 1;
          err += 2*y + 1;
        }
        if (err > 0){
          x -= 1;
          err -= 2*x + 1;
        }
    }
}

void draw_circle_delta(camera_fb_t *fb, int xc, int yc, int r, uint16_t c){
    int x = 0;
    int y = r;
    int d = 3-(2*r);
    draw_circle_parts(fb, xc, yc, x, y, c);
    while (y >= x){
        x++;
        if (d > 0){
            y--;
            d = d + 4 * (x - y) + 10;
        }else{
            d = d + 4 * x + 6;
        }
        draw_circle_parts(fb, xc, yc, x, y, c);
    }
}

static void init_ffs_and_read_index_html(void){
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    struct stat st;
    if (stat(INDEX_HTML_PATH, &st)){
        ESP_LOGE(TAG, "index.html not found");
        return;
    }

    g_index_html = calloc(1, st.st_size+1);
    g_index_html[st.st_size] = 0;

    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (fread(g_index_html, st.st_size, 1, fp) == 0){
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);
}

esp_err_t return_index_html(httpd_req_t *req){
    int response;
    response = httpd_resp_send(req, g_index_html, HTTPD_RESP_USE_STRLEN);
    return response;
}

camera_fb_t *g_fb = NULL;
esp_err_t take_picture_and_return_size(httpd_req_t *req){
    esp_err_t res = ESP_OK;

    ESP_LOGI(TAG, "take_picture_and_return_size");
    if(g_fb != NULL){
        esp_camera_fb_return(g_fb);
    }
    g_fb = esp_camera_fb_get();
    if (g_fb == NULL) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "CAM_NJPG: pf: %u, l: %u, w: %u, h: %u", g_fb->format, g_fb->len, g_fb->width, g_fb->height);

    char s[64];
    sprintf(s, "{ \"w\":\"%d\", \"h\":\"%d\" }", g_fb->width, g_fb->height);
    res = httpd_resp_send(req, (const char *) s, strlen(s));
    if(res != ESP_OK){
        ESP_LOGE(TAG, "take_picture_and_return_size %d", res);
    }
    //draw_circle_midpoint(fb, 48, 48, 10, COL_RED); 
    //bool bmp_converted = frame2bmp(fb, &_buf, &_buf_len);
    return res;
}

esp_err_t return_picture(httpd_req_t *req){
    esp_err_t res = ESP_OK;
    if(g_fb == NULL){
        return res;
    }
    ESP_LOGI(TAG, "return_picture");
    res = httpd_resp_set_type(req, "application/octet-stream");
    if(res != ESP_OK){
        ESP_LOGE(TAG, "return picture failed 2");
        return res;
    }

    //uint16_t v = 0x00f8; //little endian of red!
    //uint16_t v = 0xe007; //little endian of green!
    //uint16_t v = 0x1f00; //little endian of blue!
    //for(uint16_t i=0;i<(g_fb->len & (-1));i+=2){
    //    memcpy( ((char*)g_fb->buf) + i, &v, 2);
    //}

    res = httpd_resp_send(req, (const char *)g_fb->buf, g_fb->len);
    if(res != ESP_OK){
        ESP_LOGE(TAG, "return picture: %d", res);
    }
    //draw_circle_midpoint(fb, 48, 48, 10, COL_RED); 
    //bool bmp_converted = frame2bmp(fb, &_buf, &_buf_len);
    return res;
}

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler =  return_index_html,
    .user_ctx = NULL
};

httpd_uri_t uri_take_picture_and_return_size = {
    .uri = "/picsize.json",
    .method = HTTP_GET,
    .handler = take_picture_and_return_size,
    .user_ctx = NULL
};

httpd_uri_t uri_get_picture = {
    .uri = "/images/f2.bin",
    .method = HTTP_GET,
    .handler = return_picture,
    .user_ctx = NULL
};

httpd_handle_t setup_server(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server  = NULL;
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) == ESP_OK){
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_take_picture_and_return_size);
        httpd_register_uri_handler(server, &uri_get_picture);
    }
    return server;
}

/*
#define PORT                        CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            int to_write = len;
            while (to_write > 0) {
                int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                if (written < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                }
                to_write -= written;
            }
        }
    } while (len > 0);
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }
#ifdef CONFIG_EXAMPLE_IPV6
    else if (addr_family == AF_INET6) {
        struct sockaddr_in6 *dest_addr_ip6 = (struct sockaddr_in6 *)&dest_addr;
        bzero(&dest_addr_ip6->sin6_addr.un, sizeof(dest_addr_ip6->sin6_addr.un));
        dest_addr_ip6->sin6_family = AF_INET6;
        dest_addr_ip6->sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }
#endif

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#ifdef CONFIG_EXAMPLE_IPV6
        else if (source_addr.ss_family == PF_INET6) {
            inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}
*/

void app_main(void){
    esp_err_t err;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    err = init_camera();
    if (err != ESP_OK){
        printf("err: %s\n", esp_err_to_name(err));
        return;
    }
    init_ffs_and_read_index_html();
    setup_server();
    ESP_LOGI(TAG, "ESP32 CAM Web Server is up and running\n");


#ifdef CONFIG_EXAMPLE_IPV4
    //xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET6, 5, NULL);
#endif
}
