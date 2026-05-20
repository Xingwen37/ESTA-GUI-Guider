use std::collections::HashMap;

use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use tera::Context;

use crate::plugin::ComponentPlugin;
use crate::util::c_string_literal;

// ── Models ──

fn default_font_size() -> String {
    "ESTA_FONT_1608".into()
}

fn default_menu_items() -> Vec<MenuItemProfile> {
    Vec::new()
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct MenuItemProfile {
    pub label: String,
    pub parent_idx: u8,
    pub is_submenu: bool,
    pub event_id: u8,
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

// ── Plugin ──

fn menu_item_to_template(item: &MenuItemProfile) -> Value {
    json!({
        "label": c_string_literal(&item.label),
        "parent_idx": item.parent_idx,
        "is_submenu": item.is_submenu,
        "event_id": item.event_id,
    })
}

pub struct MenuPlugin;

impl ComponentPlugin for MenuPlugin {
    fn type_name(&self) -> &'static str {
        "menu"
    }

    fn set_defaults(&self, components: &mut HashMap<String, Value>) {
        let profile = MenuProfile {
            x_origin: 10,
            y_origin: 10,
            x_width: 160,
            y_width: 200,
            item_count: 6,
            item_height: 30,
            breadcrumb_height: 20,
            is_show_frame: true,
            is_show_breadcrumb: true,
            is_fill_background: true,
            font_size: "ESTA_FONT_1608".into(),
            theme_type: "MENU_THEME_DEFAULT".into(),
            items: vec![
                MenuItemProfile {
                    label: "Settings".into(),
                    parent_idx: 0xFF,
                    is_submenu: true,
                    event_id: 0,
                },
                MenuItemProfile {
                    label: "Display".into(),
                    parent_idx: 0,
                    is_submenu: true,
                    event_id: 0,
                },
                MenuItemProfile {
                    label: "Brightness".into(),
                    parent_idx: 1,
                    is_submenu: false,
                    event_id: 10,
                },
                MenuItemProfile {
                    label: "Backlight".into(),
                    parent_idx: 1,
                    is_submenu: false,
                    event_id: 11,
                },
                MenuItemProfile {
                    label: "Calibrate".into(),
                    parent_idx: 0xFF,
                    is_submenu: false,
                    event_id: 20,
                },
                MenuItemProfile {
                    label: "About".into(),
                    parent_idx: 0xFF,
                    is_submenu: false,
                    event_id: 30,
                },
            ],
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
                .map(|v| match serde_json::from_value::<MenuProfile>(v.clone()) {
                    Ok(p) => {
                        let items: Vec<Value> =
                            p.items.iter().map(menu_item_to_template).collect();
                        json!({
                            "x_origin": p.x_origin,
                            "y_origin": p.y_origin,
                            "x_width": p.x_width,
                            "y_width": p.y_width,
                            "item_count": p.item_count,
                            "item_height": p.item_height,
                            "breadcrumb_height": p.breadcrumb_height,
                            "is_show_frame": p.is_show_frame,
                            "is_show_breadcrumb": p.is_show_breadcrumb,
                            "is_fill_background": p.is_fill_background,
                            "font_size": p.font_size,
                            "theme_type": p.theme_type,
                            "items": items,
                            "page": p.page,
                        })
                    }
                    Err(_) => v.clone(),
                })
                .collect();
            ctx.insert(&self.profiles_key(), &values);
        }
    }
}
