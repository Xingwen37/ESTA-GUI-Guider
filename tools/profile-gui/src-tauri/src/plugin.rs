use std::collections::HashMap;

use serde_json::Value;
use tera::Context;

pub trait ComponentPlugin: Send + Sync {
    fn type_name(&self) -> &'static str;

    fn inst_count_key(&self) -> String {
        format!("{}_inst_count", self.type_name())
    }

    fn profiles_key(&self) -> String {
        format!("{}_profiles", self.type_name())
    }

    fn set_defaults(&self, components: &mut HashMap<String, Value>);

    fn fill_template_context(&self, components: &HashMap<String, Value>, ctx: &mut Context);

    fn normalize(&self, _components: &mut HashMap<String, Value>) {}
}
