use std::collections::HashMap;

use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use tera::Context;

use crate::plugin::ComponentPlugin;

// ── Models ──

fn default_font_size() -> String {
    "ESTA_FONT_1608".into()
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

// ── Plugin ──

pub struct BarChartPlugin;

impl ComponentPlugin for BarChartPlugin {
    fn type_name(&self) -> &'static str {
        "bar"
    }

    fn set_defaults(&self, components: &mut HashMap<String, Value>) {
        let profile = BarChartProfile {
            x_origin: 10,
            y_origin: 125,
            x_width: 300,
            y_width: 110,
            display_num_min: 0,
            display_num_max: 100,
            bar_count: 6,
            bar_width: 0,
            bar_spacing: 0,
            is_display_value: true,
            is_display_axis: true,
            font_size: "ESTA_FONT_1608".into(),
            theme_type: "BARCHART_THEME_DEFAULT".into(),
            page: 0,
        };
        components.insert(self.inst_count_key(), json!(1));
        components.insert(self.profiles_key(), json!([profile]));
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
                .map(|v| match serde_json::from_value::<BarChartProfile>(v.clone()) {
                    Ok(p) => json!({
                        "x_origin": p.x_origin,
                        "y_origin": p.y_origin,
                        "x_width": p.x_width,
                        "y_width": p.y_width,
                        "display_num_min": p.display_num_min,
                        "display_num_max": p.display_num_max,
                        "bar_count": p.bar_count,
                        "bar_width": p.bar_width,
                        "bar_spacing": p.bar_spacing,
                        "is_display_value": p.is_display_value,
                        "is_display_axis": p.is_display_axis,
                        "font_size": p.font_size,
                        "theme_type": p.theme_type,
                        "page": p.page,
                    }),
                    Err(_) => v.clone(),
                })
                .collect();
            ctx.insert(&self.profiles_key(), &values);
        }
    }
}
