# ER88577 MIPI DSI panel

EQT700HKY008P panel


xrandr --output DSI-1 --transform 1,0,0,0,1,60,0,0,1


## Devicetree

```
/dts-v1/;
 /plugin/;
  /{
    compatible = "rockchip,rk3566-orangepi-cm4";

    // Выводы сброса
    fragment@0 {
        target = <&pinctrl>;

        __overlay__ {
            lcd {
                lcd_rst_gpio: lcd-rst-gpio {
			        rockchip,pins = <1 1 0 &pcfg_pull_none>;
		        };
            };
        };
    };

    // Включить DSI1
 	fragment@1 {
        target = <&dsi1>;

        __overlay__ {
            status = "okay";
        };
 	};

    fragment@2 {
        target = <&video_phy1>;

        __overlay__ {
            status = "okay";
        };
    };


    fragment@3 {
        target = <&dsi1_in_vp0>;

        __overlay__ {
            status = "okay";
        };
    };

	fragment@4 {
		target = <&dsi1_in_vp1>;

		__overlay__ {
			status = "disabled";
		};
	};

    // Отключить route
    fragment@5 {
        target = <&route_dsi1>;
        __overlay__ {
            status = "okay";
        };
    };

    // Отключить HDMI
    fragment@6 {
        target = <&hdmi>;

        __overlay__ {
            status = "disabled";
        };
    };

    fragment@7 {
        target = <&i2c3>;

        __overlay__ {
            status = "disabled";
        };
    };

    fragment@8 {
        target-path = "/";

        __overlay__ {
            panel_backlight: panel-backlight {
                compatible = "pwm-backlight";
                pwms = <&pwm15 0 25000 0>;  // 40 kHz, polarity non inverted (0)
                brightness-levels = <
                    0  20  20  21  21  22  22  23
                    23  24  24  25  25  26  26  27
                    27  28  28  29  29  30  30  31
                    31  32  32  33  33  34  34  35
                    35  36  36  37  37  38  38  39
                    40  41  42  43  44  45  46  47
                    48  49  50  51  52  53  54  55
                    56  57  58  59  60  61  62  63
                    64  65  66  67  68  69  70  71
                    72  73  74  75  76  77  78  79
                    80  81  82  83  84  85  86  87
                    88  89  90  91  92  93  94  95
                    96  97  98  99 100 101 102 103
                    104 105 106 107 108 109 110 111
                    112 113 114 115 116 117 118 119
                    120 121 122 123 124 125 126 127
                    128 129 130 131 132 133 134 135
                    136 137 138 139 140 141 142 143
                    144 145 146 147 148 149 150 151
                    152 153 154 155 156 157 158 159
                    160 161 162 163 164 165 166 167
                    168 169 170 171 172 173 174 175
                    176 177 178 179 180 181 182 183
                    184 185 186 187 188 189 190 191
                    192 193 194 195 196 197 198 199
                    200 201 202 203 204 205 206 207
                    208 209 210 211 212 213 214 215
                    216 217 218 219 220 221 222 223
                    224 225 226 227 228 229 230 231
                    232 233 234 235 236 237 238 239
                    240 241 242 243 244 245 246 247
                    248 249 250 251 252 253 254 255
                >;
                default-brightness-level = <5>;
            };
        };
    };

    fragment@9 {
        target = <&pwm15>;
        __overlay__ {
            pinctrl-0 = <&pwm15m1_pins>; // GPIO4_C3
            status = "okay";
        };
    };

    fragment@10 {
        target = <&panel_test>;

        __overlay__ {
            status = "okay";

            compatible = "starry,er88577";
            backlight = <&panel_backlight>;
            reset-gpios = <&gpio1 1 1>;
            rotation = <270>;
            power-supply = <&vcc5v0_sys>;
        };
    };
};
```
