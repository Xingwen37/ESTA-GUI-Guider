use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct WaveRulerLabelProfile {
    #[serde(default)]
    pub value: f32,
}

fn default_wave_ruler_label() -> WaveRulerLabelProfile {
    WaveRulerLabelProfile { value: 0.0 }
}

fn default_wave_ruler_labels() -> [WaveRulerLabelProfile; 11] {
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
    pub ruler_y: [u16; 11],
    #[serde(default = "default_wave_ruler_labels")]
    pub ruler_label_y: [WaveRulerLabelProfile; 11],
    #[serde(default = "default_wave_ruler_unit")]
    pub ruler_unit_y: String,
    #[serde(default = "default_wave_ruler_precision")]
    pub ruler_precision_y: u8,
    pub ruler_count_y: u16,
    pub is_display_ruler_x: bool,
    pub ruler_x: [u16; 11],
    #[serde(default = "default_wave_ruler_labels")]
    pub ruler_label_x: [WaveRulerLabelProfile; 11],
    #[serde(default = "default_wave_ruler_unit")]
    pub ruler_unit_x: String,
    #[serde(default = "default_wave_ruler_precision")]
    pub ruler_precision_x: u8,
    pub ruler_count_x: u16,
    pub ruler_zero_value_x: u16,
    pub ruler_full_value_x: u16,
    #[serde(default = "default_font_size")]
    pub ruler_font_size: String,
    pub theme_type: String,
    pub is_auto_clear: bool,
    pub is_use_batch_draw: bool,
    #[serde(default)]
    pub page: u8,
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
    #[serde(default)]
    pub page: u8,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct TableColProfile {
    pub header: String,
    pub cell_type: String,
    #[serde(default)]
    pub width: u16,
    #[serde(default)]
    pub precision: u8,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct TableCellProfile {
    #[serde(default)]
    pub text: String,
    #[serde(default)]
    pub u32: u32,
    #[serde(default)]
    pub f32: f32,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct TableProfile {
    pub x_origin: u16,
    pub y_origin: u16,
    pub x_width: u16,
    pub y_width: u16,
    pub row_count: u16,
    pub col_count: u16,
    pub row_height: u16,
    #[serde(default)]
    pub is_show_header: bool,
    pub is_show_frame: bool,
    pub is_show_row_line: bool,
    #[serde(default)]
    pub is_show_col_line: bool,
    pub is_fill_background: bool,
    #[serde(default = "default_font_size")]
    pub font_size: String,
    pub theme_type: String,
    pub cols: Vec<TableColProfile>,
    #[serde(default)]
    pub cells: Vec<Vec<TableCellProfile>>,
    #[serde(default)]
    pub page: u8,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct MenuItemProfile {
    pub label: String,
    pub parent_idx: u8,
    pub is_submenu: bool,
    pub event_id: u8,
}

fn default_menu_items() -> Vec<MenuItemProfile> {
    Vec::new()
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct MenuProfile {
    pub x_origin: u16,
    pub y_origin: u16,
    pub x_width: u16,
    pub y_width: u16,
    pub item_count: u16,
    pub item_height: u16,
    pub breadcrumb_height: u16,
    pub is_show_frame: bool,
    pub is_show_breadcrumb: bool,
    pub is_fill_background: bool,
    #[serde(default = "default_font_size")]
    pub font_size: String,
    pub theme_type: String,
    #[serde(default = "default_menu_items")]
    pub items: Vec<MenuItemProfile>,
    #[serde(default)]
    pub page: u8,
}

fn default_font_size() -> String {
    "ESTA_FONT_1608".into()
}

fn default_trigger_id() -> u16 { 0xFFFF }

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct EventBinding {
    pub trigger: u8,
    pub source_id: u8,
    #[serde(default = "default_trigger_id")]
    pub trigger_id: u16,
    pub target_type: u8,
    pub target_inst: u8,
    pub action: u8,
    #[serde(default)]
    pub param: u8,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct StringEntry {
    pub target_type: u8,
    pub target_inst: u8,
    pub sub_addr: u8,
    pub text: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ActionStep {
    pub action: u8,
    pub target_type: u8,
    pub target_inst: u8,
    #[serde(default)]
    pub param: u8,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ActionSequence {
    pub step_count: u8,
    pub steps: Vec<ActionStep>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ProfileSet {
    pub wave_inst_count: u16,
    pub bar_inst_count: u16,
    #[serde(default)]
    pub table_inst_count: u16,
    #[serde(default)]
    pub menu_inst_count: u16,
    pub button_count: u16,
    #[serde(default = "default_page_count")]
    pub page_count: u8,
    #[serde(default)]
    pub binding_count: u8,
    pub wave_profiles: Vec<WaveProfile>,
    pub bar_profiles: Vec<BarChartProfile>,
    #[serde(default)]
    pub table_profiles: Vec<TableProfile>,
    #[serde(default)]
    pub menu_profiles: Vec<MenuProfile>,
    #[serde(default)]
    pub bindings: Vec<EventBinding>,
    #[serde(default)]
    pub string_count: u8,
    #[serde(default)]
    pub strings: Vec<StringEntry>,
    #[serde(default)]
    pub sequence_count: u8,
    #[serde(default)]
    pub sequences: Vec<ActionSequence>,
}

fn default_page_count() -> u8 { 1 }

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
