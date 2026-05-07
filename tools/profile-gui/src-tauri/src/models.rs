use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct OscProfile {
    pub x_origin: u16,
    pub y_origin: u16,
    pub x_width: u16,
    pub y_width: u16,
    pub display_num_min: u16,
    pub display_num_max: u16,
    pub channel_num: u16,
    pub channel_mask: u8,
    pub is_display_ruler_y: bool,
    pub ruler_y: [u16; 5],
    pub ruler_count_y: u16,
    pub ruler_num_digits_y: u16,
    pub is_display_ruler_x: bool,
    pub ruler_x: [u16; 5],
    pub ruler_count_x: u16,
    pub ruler_zero_value_x: u16,
    pub ruler_full_value_x: u16,
    pub ruler_num_digits_x: u16,
    pub theme_type: String,
    pub is_auto_clear: bool,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ProfileSet {
    pub osc_count: u16,
    pub profiles: Vec<OscProfile>,
}

impl OscProfile {
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
