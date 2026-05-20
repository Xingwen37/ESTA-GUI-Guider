use std::collections::HashMap;

use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use tera::Context;

use crate::plugin::ComponentPlugin;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct SoftTimerConfig {
    pub period_ms: u16,
    pub event_type: u8,
    pub event_source: u8,
    pub event_id: u16,
}

pub struct SoftTimerPlugin;

impl ComponentPlugin for SoftTimerPlugin {
    fn type_name(&self) -> &'static str {
        "soft_timer"
    }

    fn inst_count_key(&self) -> String {
        "timer_count".to_string()
    }

    fn profiles_key(&self) -> String {
        "timer_configs".to_string()
    }

    fn set_defaults(&self, components: &mut HashMap<String, Value>) {
        components.insert(self.inst_count_key(), json!(0));
        components.insert(self.profiles_key(), json!([]));
    }

    fn fill_template_context(&self, components: &HashMap<String, Value>, ctx: &mut Context) {
        let count = components
            .get(&self.inst_count_key())
            .and_then(|v| v.as_u64())
            .unwrap_or(0);
        ctx.insert(&self.inst_count_key(), &count);

        if let Some(raw) = components.get(&self.profiles_key()) {
            let values: Vec<Value> = raw
                .as_array()
                .into_iter()
                .flatten()
                .map(|v| match serde_json::from_value::<SoftTimerConfig>(v.clone()) {
                    Ok(t) => json!({
                        "period_ms": t.period_ms,
                        "event_type": t.event_type,
                        "event_source": t.event_source,
                        "event_id": t.event_id,
                    }),
                    Err(_) => v.clone(),
                })
                .collect();
            ctx.insert(&self.profiles_key(), &values);
        }
    }
}
