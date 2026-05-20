use std::collections::HashMap;

use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use tera::Context;

use crate::plugin::ComponentPlugin;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct FlagConfig {
    pub mode: u8,
    pub event_type: u8,
    pub event_source: u8,
    pub event_id: u16,
}

pub struct FlagPlugin;

impl ComponentPlugin for FlagPlugin {
    fn type_name(&self) -> &'static str {
        "flag"
    }

    fn set_defaults(&self, components: &mut HashMap<String, Value>) {
        let defaults: Vec<Value> = (0..8)
            .map(|_| {
                json!({
                    "mode": 0,
                    "event_type": 0,
                    "event_source": 0,
                    "event_id": 0,
                })
            })
            .collect();
        components.insert(self.inst_count_key(), json!(8));
        components.insert(self.profiles_key(), json!(defaults));
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
                .map(|v| match serde_json::from_value::<FlagConfig>(v.clone()) {
                    Ok(f) => json!({
                        "mode": f.mode,
                        "event_type": f.event_type,
                        "event_source": f.event_source,
                        "event_id": f.event_id,
                    }),
                    Err(_) => v.clone(),
                })
                .collect();
            ctx.insert(&self.profiles_key(), &values);
        }
    }
}
