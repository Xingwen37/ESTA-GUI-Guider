use std::collections::HashMap;

use crate::models::{EventBinding, ProfileSet};
use crate::registry::Registry;

pub fn default_profile(registry: &Registry) -> ProfileSet {
    let mut components = HashMap::new();
    for plugin in registry.plugins() {
        plugin.set_defaults(&mut components);
    }

    ProfileSet {
        button_count: 4,
        page_count: 1,
        binding_count: 6,
        bindings: vec![
            EventBinding { trigger: 1, source_id: 0, trigger_id: 0xFFFF, target_type: 0, target_inst: 0, action: 3, param: 0, guard_and_mask: 0, guard_or_mask: 0, guard_inv_mask: 0 },
            EventBinding { trigger: 1, source_id: 1, trigger_id: 0xFFFF, target_type: 4, target_inst: 0, action: 1, param: 0, guard_and_mask: 0, guard_or_mask: 0, guard_inv_mask: 0 },
            EventBinding { trigger: 1, source_id: 2, trigger_id: 0xFFFF, target_type: 0, target_inst: 0, action: 4, param: 0, guard_and_mask: 0, guard_or_mask: 0, guard_inv_mask: 0 },
            EventBinding { trigger: 1, source_id: 3, trigger_id: 0xFFFF, target_type: 3, target_inst: 0, action: 5, param: 0, guard_and_mask: 0, guard_or_mask: 0, guard_inv_mask: 0 },
            EventBinding { trigger: 1, source_id: 4, trigger_id: 0xFFFF, target_type: 3, target_inst: 0, action: 6, param: 0, guard_and_mask: 0, guard_or_mask: 0, guard_inv_mask: 0 },
            EventBinding { trigger: 1, source_id: 5, trigger_id: 0xFFFF, target_type: 3, target_inst: 0, action: 7, param: 0, guard_and_mask: 0, guard_or_mask: 0, guard_inv_mask: 0 },
        ],
        string_count: 0,
        strings: vec![],
        sequence_count: 0,
        sequences: vec![],
        components,
    }
}
