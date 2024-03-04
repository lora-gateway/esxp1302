set(libloragw_src
    "libloragw/loragw_ad5338r.c"
    "libloragw/loragw_aux.c"
    "libloragw/loragw_cal.c"
    "libloragw/loragw_com.c"
    "libloragw/loragw_debug.c"
    "libloragw/loragw_gpio.c"
    "libloragw/loragw_gps.c"
    "libloragw/loragw_hal.c"
    "libloragw/loragw_i2c.c"
    "libloragw/loragw_lbt.c"
    "libloragw/loragw_reg.c"
    "libloragw/loragw_spi.c"
    "libloragw/loragw_stts751.c"
    "libloragw/loragw_sx1250.c"
    "libloragw/loragw_sx125x.c"
    "libloragw/loragw_sx1261.c"
    "libloragw/loragw_sx1302.c"
    "libloragw/loragw_sx1302_rx.c"
    "libloragw/loragw_sx1302_timestamp.c"
    "libloragw/loragw_usb.c"
    "libloragw/sx1250_com.c"
    "libloragw/sx1250_spi.c"
    "libloragw/sx1250_usb.c"
    "libloragw/sx125x_com.c"
    "libloragw/sx125x_spi.c"
    "libloragw/sx1261_com.c"
    "libloragw/sx1261_spi.c"
    "libloragw/sx1261_usb.c"
)

set(libtools_src
    "libtools/base64.c"
    "libtools/parson.c"
    "libtools/tinymt32.c"
)

if(CONFIG_LIBLORAGW_TEST)
    set(libloragw_test_src
        "libloragw-test/test_loragw_toa.c"
        "libloragw-test/test_loragw_hal_tx.c"
        "libloragw-test/test_loragw_hal_rx.c"
        "libloragw-test/cli4test.c"
    )
    set(pkt_fwd_src "")
else()
    set(libloragw_test_src "")
    set(pkt_fwd_src
	"packet_forwarder/jitqueue.c"
	"packet_forwarder/lora_pkt_fwd.c"
        "packet_forwarder/ioe.c"
        "packet_forwarder/led_indication.c"
        "packet_forwarder/web_config.c"
        "packet_forwarder/http_server.c"
    )
endif()


idf_component_register(SRCS
    "${libtools_src}"
    "${libloragw_src}"
    "${libloragw_test_src}"
    "${pkt_fwd_src}"
    "${main_src}"
    INCLUDE_DIRS "libloragw"
    INCLUDE_DIRS "libtools"
    INCLUDE_DIRS "libloragw-test"
    INCLUDE_DIRS "packet_forwarder"
)
