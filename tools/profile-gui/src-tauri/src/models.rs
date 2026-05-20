use std::collections::HashMap;

use serde::{Deserialize, Serialize};

fn default_page_count() -> u8 {
    1
}

fn default_trigger_id() -> u16 {
    0xFFFF
}

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
    pub button_count: u16,
    #[serde(default = "default_page_count")]
    pub page_count: u8,
    #[serde(default)]
    pub binding_count: u8,
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

    #[serde(flatten)]
    pub components: HashMap<String, serde_json::Value>,
}
