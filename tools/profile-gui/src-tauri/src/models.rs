use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct WaveRulerLabelProfile {
    pub value_type: String,
    pub int_value: i32,
    pub float_value: f32,
}

fn default_wave_ruler_label() -> WaveRulerLabelProfile {
    WaveRulerLabelProfile {
        value_type: "WAVE_RULER_LABEL_INT".into(),
        int_value: 0,
        float_value: 0.0,
    }
}

fn default_wave_ruler_labels() -> [WaveRulerLabelProfile; 10] {
    std::array::from_fn(|_| default_wave_ruler_label())
}

fn default_wave_ruler_unit() -> String {
    String::new()
}

fn default_wave_ruler_precision() -> u8 {
    0
}

fn default_wave_x_scale() -> u16 {
    1
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct WaveProfile {
    pub x_origin: u16,
    pub y_origin: u16,
    pub x_width: u16,
    pub y_width: u16,
    pub display_num_min: u16,
    pub display_num_max: u16,
    #[serde(default = "default_wave_x_scale")]
    pub x_scale: u16,
    pub channel_num: u16,
    pub channel_mask: u8,
    pub is_display_ruler_y: bool,
    pub ruler_y: [u16; 10],
    #[serde(default = "default_wave_ruler_labels")]
    pub ruler_label_y: [WaveRulerLabelProfile; 10],
    #[serde(default = "default_wave_ruler_unit")]
    pub ruler_unit_y: String,
    #[serde(default = "default_wave_ruler_precision")]
    pub ruler_precision_y: u8,
    pub ruler_count_y: u16,
    pub ruler_num_digits_y: u16,
    #[serde(default = "default_font_size")]
    pub ruler_font_size_y: String,
    pub is_display_ruler_x: bool,
    pub ruler_x: [u16; 10],
    #[serde(default = "default_wave_ruler_labels")]
    pub ruler_label_x: [WaveRulerLabelProfile; 10],
    #[serde(default = "default_wave_ruler_unit")]
    pub ruler_unit_x: String,
    #[serde(default = "default_wave_ruler_precision")]
    pub ruler_precision_x: u8,
    pub ruler_count_x: u16,
    pub ruler_zero_value_x: u16,
    pub ruler_full_value_x: u16,
    pub ruler_num_digits_x: u16,
    #[serde(default = "default_font_size")]
    pub ruler_font_size_x: String,
    pub theme_type: String,
    pub is_auto_clear: bool,
    pub is_use_batch_draw: bool,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct BarChartProfile {
    pub x_origin: u16,
    pub y_origin: u16,
    pub x_width: u16,
    pub y_width: u16,
    pub display_num_min: u16,
    pub display_num_max: u16,
    pub bar_count: u16,
    pub bar_width: u16,
    pub bar_spacing: u16,
    pub is_display_value: bool,
    pub is_display_axis: bool,
    #[serde(default = "default_font_size")]
    pub font_size: String,
    pub theme_type: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct TableRowProfile {
    pub label: String,
    pub value_kind: String,
    pub number_type: String,
    pub unit: String,
    pub precision: u8,
    pub default_u32: u32,
    pub default_float: f32,
    pub default_text: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct TableProfile {
    pub x_origin: u16,
    pub y_origin: u16,
    pub x_width: u16,
    pub y_width: u16,
    pub row_count: u16,
    pub row_height: u16,
    pub label_col_width: u16,
    pub value_col_width: u16,
    pub unit_col_width: u16,
    pub is_auto_col_width: bool,
    pub is_show_frame: bool,
    pub is_show_row_line: bool,
    pub is_fill_background: bool,
    #[serde(default = "default_font_size")]
    pub font_size: String,
    pub theme_type: String,
    pub rows: Vec<TableRowProfile>,
}

fn default_font_size() -> String {
    "ESTA_FONT_1608".into()
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ProfileSet {
    pub wave_inst_count: u16,
    pub bar_inst_count: u16,
    #[serde(default)]
    pub table_inst_count: u16,
    pub button_count: u16,
    pub wave_profiles: Vec<WaveProfile>,
    pub bar_profiles: Vec<BarChartProfile>,
    #[serde(default)]
    pub table_profiles: Vec<TableProfile>,
}

impl WaveProfile {
    pub fn channel_mask_expr(&self) -> String {
        let terms: Vec<String> = (0..8)
            .filter(|i| (self.channel_mask & (1u8 << i)) != 0)
            .map(|i| format!("CH{}", i))
            .collect();
        if terms.is_empty() {
            "NO_CHANNELS_MASK".into()
        } else {
            terms.join(" | ")
        }
    }
}
