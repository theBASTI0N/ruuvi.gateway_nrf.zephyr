# ruuvi.gateway_nrf.zephyr
Ruuvi Gateway BLE scanner based on Zephyr OS

This has been tested and built usig Zephyr OS v2.2.99.

Installation instructions for preparing Zephyr OS can be found at:
https://docs.zephyrproject.org/latest/getting_started/index.html

# Cloning and building
This assumes that you are running linux and followed the instructions found at the link above and cloned zephyr into ~/zephyrproject.

```bash
cd ~/zephyrproject/zephyr
mkdir applications && cd applications
git clone https://github.com/theBASTI0N/ruuvi.gateway_nrf.zephyr.git
cd ruuvi.gateway_nrf.zephyr
west build
```