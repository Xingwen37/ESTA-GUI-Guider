use std::path::Path;

use serde_json::json;
use tauri::State;

use crate::models::ProfileSet;
use crate::registry::Registry;

use super::{AppState, PROFILE_C, PROFILE_JSON, TEMPLATE};

pub fn save_profile_internal(
    repo_root: &Path,
    tera: &tera::Tera,
    registry: &Registry,
    data: &ProfileSet,
) -> Result<(), String> {
    let json_path = repo_root.join(PROFILE_JSON);
    let c_path = repo_root.join(PROFILE_C);

    let bindings_for_template: Vec<serde_json::Value> = data
        .bindings
        .iter()
        .map(|b| {
            json!({
                "trigger": b.trigger, "source_id": b.source_id, "trigger_id": b.trigger_id,
                "target_type": b.target_type, "target_inst": b.target_inst,
                "action": b.action, "param": b.param,
                "guard_and_mask": b.guard_and_mask,
                "guard_or_mask": b.guard_or_mask,
                "guard_inv_mask": b.guard_inv_mask,
            })
        })
        .collect();

    let strings_for_template: Vec<serde_json::Value> = data
        .strings
        .iter()
        .map(|s| json!({ "sub_addr": s.sub_addr, "text": s.text }))
        .collect();

    let sequences_for_template: Vec<serde_json::Value> = data
        .sequences
        .iter()
        .map(|seq| {
            let steps: Vec<serde_json::Value> = seq
                .steps
                .iter()
                .map(|step| {
                    json!({
                        "action": step.action, "target_type": step.target_type,
                        "target_inst": step.target_inst, "param": step.param,
                    })
                })
                .collect();
            json!({ "step_count": seq.step_count, "steps": steps })
        })
        .collect();

    let mut ctx = tera::Context::new();
    ctx.insert("button_count", &data.button_count);
    ctx.insert("page_count", &data.page_count);
    ctx.insert("binding_count", &data.bindings.len());
    ctx.insert("bindings", &bindings_for_template);
    ctx.insert("string_count", &data.strings.len());
    ctx.insert("strings", &strings_for_template);
    ctx.insert("sequence_count", &data.sequences.len());
    ctx.insert("sequences", &sequences_for_template);

    for plugin in registry.plugins() {
        plugin.fill_template_context(&data.components, &mut ctx);
    }

    let c_code = tera
        .render(TEMPLATE, &ctx)
        .map_err(|e| format!("模板渲染失败: {}", e))?;

    if c_path.exists() {
        let backup = c_path.with_extension("c.bak");
        std::fs::copy(&c_path, &backup).map_err(|e| e.to_string())?;
    }
    std::fs::write(&c_path, c_code).map_err(|e| e.to_string())?;

    let json = serde_json::to_string_pretty(&data).map_err(|e| e.to_string())?;
    std::fs::write(&json_path, json).map_err(|e| e.to_string())?;

    Ok(())
}

#[tauri::command]
pub fn save_profile(state: State<AppState>, data: ProfileSet) -> Result<(), String> {
    save_profile_internal(
        &state.repo_root,
        &state.tera,
        &state.registry,
        &data,
    )
}

#[tauri::command]
pub fn load_profile(state: State<AppState>) -> Result<ProfileSet, String> {
    let json_path = state.repo_root.join(PROFILE_JSON);
    if json_path.exists() {
        let mut content = std::fs::read_to_string(&json_path).map_err(|e| e.to_string())?;
        if content.starts_with('\u{FEFF}') {
            content.remove(0);
        }
        let mut profile: ProfileSet =
            serde_json::from_str(&content).map_err(|e| format!("JSON 解析失败: {}", e))?;
        for plugin in state.registry.plugins() {
            plugin.normalize(&mut profile.components);
        }
        Ok(profile)
    } else {
        Ok(crate::defaults::default_profile(&state.registry))
    }
}
