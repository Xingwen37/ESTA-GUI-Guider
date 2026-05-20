use crate::plugin::ComponentPlugin;

pub struct Registry {
    plugins: Vec<Box<dyn ComponentPlugin>>,
}

impl Registry {
    pub fn new() -> Self {
        Registry {
            plugins: Vec::new(),
        }
    }

    pub fn register(&mut self, plugin: Box<dyn ComponentPlugin>) {
        self.plugins.push(plugin);
    }

    pub fn plugins(&self) -> &[Box<dyn ComponentPlugin>] {
        &self.plugins
    }
}
