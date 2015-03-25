BehaviorFooClass := Object [

	foo [

	]

]

[
	Assert true: BehaviorFooClass superClass == Object.
	Assert true: (Object subClasses includes: BehaviorFooClass).
	Assert true: (BehaviorFooClass methodDictionary size = 1).
	Assert true: (BehaviorFooClass methodDictionary includesKey: #foo).
	Assert true: (BehaviorFooClass inheritsFrom: Object).
	Assert false: (BehaviorFooClass inheritsFrom: UndefinedObject).
	Assert true: (BehaviorFooClass selectors includes: #foo).
	Assert true: (BehaviorFooClass allSelectors includes: #foo).
	Assert true: (BehaviorFooClass allSelectors includes: #isNil).
	Assert true: (BehaviorFooClass compiledMethodAt: #foo) notNil.

	Assert do: [Object new: Object] expect: Error.
]
