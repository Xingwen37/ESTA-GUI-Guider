use std::collections::HashMap;

use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use tera::Context;

use crate::plugin::ComponentPlugin;
use crate::util::c_string_literal;

// ── Models ──

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

fn default_font_size() -> String {
    "ESTA_FONT_1608".into()
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

// ── Helpers ──

fn wave_label_from_u16(value: u16) -> WaveRulerLabelProfile {
    WaveRulerLabelProfile {
        value: f32::from(value),
    }
}

pub fn wave_labels_from_positions(values: &[u16; 11]) -> [WaveRulerLabelProfile; 11] {
    std::array::from_fn(|i| wave_label_from_u16(values[i]))
}

fn wave_labels_for_template(labels: &[WaveRulerLabelProfile; 11]) -> Vec<Value> {
    labels.iter().map(|l| json!({ "value": l.value })).collect()
}

fn channel_mask_expr(mask: u8) -> String {
    let terms: Vec<String> = (0..8)
        .filter(|i| (mask & (1u8 << i)) != 0)
        .map(|i| format!("CH{}", i))
        .collect();
    if terms.is_empty() {
        "NO_CHANNELS_MASK".into()
    } else {
        terms.join(" | ")
    }
}

// ── Plugin ──

pub struct WavePlugin;

impl ComponentPlugin for WavePlugin {
    fn type_name(&self) -> &'static str {
        "wave"
    }

    fn set_defaults(&self, components: &mut HashMap<String, Value>) {
        let ruler_y = [1000, 2000, 3000, 4000, 0, 0, 0, 0, 0, 0, 0];
        let ruler_x = [30, 50, 90, 0, 0, 0, 0, 0, 0, 0, 0];

        let base = WaveProfile {
            x_origin: 10,
            y_origin: 0,
            x_width: 200,
            y_width: 120,
            display_num_min: 0,
            display_num_max: 4095,
            x_scale: 1,
            channel_num: 4,
            channel_mask: 0b00001111,
            is_display_ruler_y: true,
            ruler_y,
            ruler_label_y: wave_labels_from_positions(&ruler_y),
            ruler_unit_y: String::new(),
            ruler_precision_y: 0,
            ruler_count_y: 4,
            is_display_ruler_x: true,
            ruler_x,
            ruler_label_x: wave_labels_from_positions(&ruler_x),
            ruler_unit_x: String::new(),
            ruler_precision_x: 0,
            ruler_count_x: 3,
            ruler_zero_value_x: 0,
            ruler_full_value_x: 100,
            ruler_font_size: "ESTA_FONT_1608".into(),
            theme_type: "WAVE_THEME_DEFAULT".into(),
            is_auto_clear: true,
            is_use_batch_draw: false,
            page: 0,
        };

        let light = WaveProfile {
            x_origin: 10,
            y_origin: 0,
            theme_type: "WAVE_THEME_LIGHT".into(),
            is_use_batch_draw: true,
            ..base.clone()
        };

        components.insert(self.inst_count_key(), json!(2));
        components.insert(self.profiles_key(), json!([light, base]));
    }

    fn fill_template_context(
        &self,
        components: &HashMap<String, Value>,
        ctx: &mut Context,
    ) {
        if let Some(count) = components.get(&self.inst_count_key()) {
            ctx.insert(&self.inst_count_key(), count);
        }

        if let Some(raw) = components.get(&self.profiles_key()) {
            let values: Vec<Value> = raw
                .as_array()
                .into_iter()
                .flatten()
                .map(|v| match serde_json::from_value::<WaveProfile>(v.clone()) {
                    Ok(p) => json!({
                        "x_origin": p.x_origin,
                        "y_origin": p.y_origin,
                        "x_width": p.x_width,
                        "y_width": p.y_width,
                        "display_num_min": p.display_num_min,
                        "display_num_max": p.display_num_max,
                        "x_scale": p.x_scale,
                        "channel_num": p.channel_num,
                        "channel_mask_expr": channel_mask_expr(p.channel_mask),
                        "is_display_ruler_y": p.is_display_ruler_y,
                        "ruler_y": p.ruler_y,
                        "ruler_label_y": wave_labels_for_template(&p.ruler_label_y),
                        "ruler_unit_y": c_string_literal(&p.ruler_unit_y),
                        "ruler_precision_y": p.ruler_precision_y,
                        "ruler_count_y": p.ruler_count_y,
                        "is_display_ruler_x": p.is_display_ruler_x,
                        "ruler_x": p.ruler_x,
                        "ruler_label_x": wave_labels_for_template(&p.ruler_label_x),
                        "ruler_unit_x": c_string_literal(&p.ruler_unit_x),
                        "ruler_precision_x": p.ruler_precision_x,
                        "ruler_count_x": p.ruler_count_x,
                        "ruler_zero_value_x": p.ruler_zero_value_x,
                        "ruler_full_value_x": p.ruler_full_value_x,
                        "ruler_font_size": p.ruler_font_size,
                        "theme_type": p.theme_type,
                        "is_auto_clear": p.is_auto_clear,
                        "is_use_batch_draw": p.is_use_batch_draw,
                        "page": p.page,
                    }),
                    Err(_) => v.clone(),
                })
                .collect();
            ctx.insert(&self.profiles_key(), &values);
        }
    }

    fn normalize(&self, components: &mut HashMap<String, Value>) {
        let Some(Value::Array(profiles)) = components.get_mut(&self.profiles_key()) else {
            return;
        };

        let first = profiles.first().and_then(|v| v.as_object());
        let has_label_y = first.map(|o| o.contains_key("ruler_label_y")).unwrap_or(true);
        let has_label_x = first.map(|o| o.contains_key("ruler_label_x")).unwrap_or(true);

        if has_label_y && has_label_x {
            return;
        }

        for profile in profiles {
            let Some(obj) = profile.as_object_mut() else { continue };

            if !has_label_y {
                let labels: Vec<Value> = obj
                    .get("ruler_y")
                    .and_then(|v| v.as_array())
                    .map(|arr| {
                        arr.iter()
                            .map(|v| json!({ "value": v.as_u64().map(|n| n as f32).unwrap_or(0.0) }))
                            .collect()
                    })
                    .unwrap_or_default();
                obj.insert("ruler_label_y".into(), Value::Array(labels));
                obj.insert("ruler_unit_y".into(), Value::String(String::new()));
                obj.insert("ruler_precision_y".into(), Value::Number(0.into()));
            }

            if !has_label_x {
                let labels: Vec<Value> = obj
                    .get("ruler_x")
                    .and_then(|v| v.as_array())
                    .map(|arr| {
                        arr.iter()
                            .map(|v| json!({ "value": v.as_u64().map(|n| n as f32).unwrap_or(0.0) }))
                            .collect()
                    })
                    .unwrap_or_default();
                obj.insert("ruler_label_x".into(), Value::Array(labels));
                obj.insert("ruler_unit_x".into(), Value::String(String::new()));
                obj.insert("ruler_precision_x".into(), Value::Number(0.into()));
            }
        }
    }
}
