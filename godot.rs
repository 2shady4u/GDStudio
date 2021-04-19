use std::io;

fn main() {
    println!("Hello, world!");
}

#[derive(Debug)]
pub struct SomeClass {
	var: i32,
}

impl SomeClass {
	pub fn new() -> Self {
		SomeClass {
			var: 0,
		}
	}
	
	fn some_method(&mut self) {
		println!("Some Class method");
	}
}