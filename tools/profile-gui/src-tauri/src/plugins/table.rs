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

// ── Plugin ──

fn col_to_template(col: &TableColProfile) -> Value {
    json!({
        "header": c_string_literal(&col.header),
        "cell_type": col.cell_type,
        "width": col.width,
        "precision": col.precision,
    })
}

fn cell_to_template(cell: &TableCellProfile, col_type: &str) -> Value {
    let (active_field, active_value) = match col_type {
        "TABLE_CELL_UINT32" => ("u32".to_string(), format!("{}", cell.u32)),
        "TABLE_CELL_FLOAT" => ("f32".to_string(), format!("{:.6}", cell.f32)),
        _ => ("text".to_string(), c_string_literal(&cell.text)),
    };
    json!({ "active_field": active_field, "active_value": active_value })
}

pub struct TablePlugin;

impl ComponentPlugin for TablePlugin {
    fn type_name(&self) -> &'static str {
        "table"
    }

    fn set_defaults(&self, components: &mut HashMap<String, Value>) {
        let profile = TableProfile {
            x_origin: 210,
            y_origin: 0,
            x_width: 110,
            y_width: 72,
            row_count: 2,
            col_count: 3,
            row_height: 20,
            is_show_header: false,
            is_show_frame: true,
            is_show_row_line: false,
            is_show_col_line: false,
            is_fill_background: true,
            font_size: "ESTA_FONT_1608".into(),
            theme_type: "TABLE_THEME_LIGHT".into(),
            cols: vec![
                TableColProfile {
                    header: "Name".into(),
                    cell_type: "TABLE_CELL_TEXT".into(),
                    width: 0,
                    precision: 0,
                },
                TableColProfile {
                    header: "Value".into(),
                    cell_type: "TABLE_CELL_UINT32".into(),
                    width: 0,
                    precision: 0,
                },
                TableColProfile {
                    header: "Unit".into(),
                    cell_type: "TABLE_CELL_TEXT".into(),
                    width: 0,
                    precision: 0,
                },
            ],
            cells: vec![
                vec![
                    TableCellProfile {
                        text: "Vpp".into(),
                        u32: 0,
                        f32: 0.0,
                    },
                    TableCellProfile {
                        text: String::new(),
                        u32: 1000,
                        f32: 0.0,
                    },
                    TableCellProfile {
                        text: "mV".into(),
                        u32: 0,
                        f32: 0.0,
                    },
                ],
                vec![
                    TableCellProfile {
                        text: "Fre".into(),
                        u32: 0,
                        f32: 0.0,
                    },
                    TableCellProfile {
                        text: String::new(),
                        u32: 1230,
                        f32: 0.0,
                    },
                    TableCellProfile {
                        text: "Hz".into(),
                        u32: 0,
                        f32: 0.0,
                    },
                ],
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
                .map(|v| match serde_json::from_value::<TableProfile>(v.clone()) {
                    Ok(p) => {
                        let cols: Vec<Value> = p.cols.iter().map(col_to_template).collect();
                        let cells: Vec<Value> = p
                            .cells
                            .iter()
                            .map(|row| {
                                let row_cells: Vec<Value> = row
                                    .iter()
                                    .enumerate()
                                    .map(|(ci, cell)| {
                                        let ct = p
                                            .cols
                                            .get(ci)
                                            .map(|c| c.cell_type.as_str())
                                            .unwrap_or("TABLE_CELL_TEXT");
                                        cell_to_template(cell, ct)
                                    })
                                    .collect();
                                json!(row_cells)
                            })
                            .collect();
                        json!({
                            "x_origin": p.x_origin,
                            "y_origin": p.y_origin,
                            "x_width": p.x_width,
                            "y_width": p.y_width,
                            "row_count": p.row_count,
                            "col_count": p.col_count,
                            "row_height": p.row_height,
                            "is_show_header": p.is_show_header,
                            "is_show_frame": p.is_show_frame,
                            "is_show_row_line": p.is_show_row_line,
                            "is_show_col_line": p.is_show_col_line,
                            "is_fill_background": p.is_fill_background,
                            "font_size": p.font_size,
                            "theme_type": p.theme_type,
                            "cols": cols,
                            "cells": cells,
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
