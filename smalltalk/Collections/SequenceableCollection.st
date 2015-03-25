SequenceableCollection := Collection [

	"comparing"

	hash [
		| hash |

		hash := self class hash.
		self do: [ :v | hash := hash bitXor: v hash].
		^hash
	]


	= aCollection [
		| size |

		self class == aCollection class ifFalse: [^false].
		(size := self size) = aCollection size ifFalse: [^false].

		1 to: size do: [ :i |
			(self at: i) = (aCollection at: i) ifFalse: [^false]].
		^true
	]


	startsWith: aSequenceableCollection [
		aSequenceableCollection size > self size ifTrue: [^false].
		aSequenceableCollection keysAndValuesDo: [ :i :c |
			(self at: i) = c ifFalse: [^false]].
		^true
	]


	"enumerating"

	do: aBlock [
		1 to: self size do: [ :i | aBlock value: (self at: i)].
	]


	reverseDo: aBlock [
		self size to: 1 by: -1 do: [ :i | aBlock value: (self at: i)].
	]


	keysAndValuesDo: aBlock [
		1 to: self size do: [ :i | aBlock value: i value: (self at: i)].
	]


	includes: anObject [
		1 to: self size do: [ :index | anObject = (self at: index) ifTrue: [^true]].
		^false
	]


	collect: aBlock [
		| newCollection |

		newCollection := self class new: self size.
		1 to: self size do: [ :index | newCollection at: index put: (aBlock value: (self at: index))].
		^newCollection
	]


	"accessing"

	size [
		SubClassResponsibility signal.
	]


	first [
		^self at: 1
	]


	last [
		^self at: self size
	]


	indexOf: anObject [
		^self indexOf: anObject ifAbsent: [0]
	]


	indexOf: anObject ifAbsent: aBlock [
		^self indexOf: anObject startingAt: 1 ifAbsent: [0]
	]


	indexOf: anObject startingAt: anInteger ifAbsent: aBlock [
		anInteger to: self size do: [ :i |
			(self at: i) = anObject ifTrue: [^i]].
		^aBlock value
	]


	lastIndexOf: anObject [
		self size to: 1 by: -1 do: [ :i |
			(self at: i) = anObject ifTrue: [^i]].
		^0
	]


	indexOfSubCollection: aCollection startingAt: anInteger [
		^self
			indexOfSubCollection: aCollection
			startingAt: anInteger
			ifAbsent: [0]
	]


	indexOfSubCollection: aCollection startingAt: anInteger ifAbsent: aBlock [
		| result first |

		aCollection size = 0 ifTrue: [^aBlock value].
		first := aCollection at: 1.

		anInteger to: self size - aCollection size + 1 do: [ :i |
			result := true.
			(self at: i) = first ifTrue: [
				1 to: aCollection size do: [ :j |
					result := (self at: i + j - 1) = (aCollection at: j)].
			result ifTrue: [^i]]].

		^aBlock value
	]


	replaceFrom: start to: stop with: replacement [
		self replaceFrom: start to: stop with: replacement startingAt: 1.
	]


	replaceFrom: start to: stop with: replacement startingAt: replacementStart [
		| delta |

		delta := replacementStart - start.
		start to: stop do: [ :i | self at: i put: (replacement at: i + delta)].
	]


	splitBy: anObject [
		| collection last |

		collection := OrderedCollection new.
		last := 1.

		1 to: self size do: [ :i |
			(self at: i) = anObject ifTrue: [
				last = i ifFalse: [collection addLast: (self copyFrom: last to: i - 1)].
				last := i + 1.
			].
		].
		last <= self size ifTrue: [collection addLast: (self copyFrom: last to: self size)].
		^collection
	]


	splitByAll: aCollection[
		| collection last |

		aCollection size = 1 ifTrue: [^self splitBy: aCollection first].
		collection := OrderedCollection new.
		last := 1.

		1 to: self size do: [ :i |
			(aCollection includes: (self at: i)) ifTrue: [
				last = i ifFalse: [collection addLast: (self copyFrom: last to: i - 1)].
				last := i + 1.
			].
		].
		last <= self size ifTrue: [collection addLast: (self copyFrom: last to: self size)].
		^collection
	]


	"removing"

	remove: anObject ifAbsent: aBlock [
		ShouldNotImplement signal.
	]


	"copying"

	, aCollection [
		^self copyReplaceFrom: self size + 1
			to: self size
			with: aCollection
	]


	copyFrom: start [
		^self copyFrom: start to: self size
	]


	copyFrom: start to: stop [
		| newSize |

		newSize := stop - start + 1.
		^(self species new: newSize)
			replaceFrom: 1
			to: newSize
			with: self
			startingAt: start
	]


	copyResized: anInteger [
		| newCollection |

		newCollection := self species new: anInteger.
		newCollection
			replaceFrom: 1
			to: (self size min: anInteger)
			with: self
			startingAt: 1.
		^newCollection
	]


	copyReplaceFrom: start to: stop with: replacementCollection [
		| newCollection newSize endReplacement |

		newSize := self size - (stop - start + 1) + replacementCollection size.
		endReplacement := start - 1 + replacementCollection size.
		newCollection := self species new: newSize.

		newCollection
			replaceFrom: 1
			to: start - 1
			with: self
			startingAt: 1.
		newCollection
			replaceFrom: start
			to: endReplacement
			with: replacementCollection
			startingAt: 1.
		newCollection
			replaceFrom: endReplacement + 1
			to: newSize
			with: self
			startingAt: stop + 1.
		^newCollection
	]


	copyWith: anObject [
		| newCollection newSize |

		newSize := self size + 1.
		newCollection := self copyResized: newSize.
		newCollection at: newSize put: anObject.
		^newCollection
	]


	copyUpTo: anObject [
		^self
			copyFrom: 1
			to: (self indexOf: anObject ifAbsent: [self size + 1]) - 1
	]


	copyAfter: anObject [
		^self copyFrom: (self indexOf: anObject ifAbsent: [self size]) + 1
	]


	"converting"

	asArray [
		^(Array new: self size) replaceFrom: 1 to: self size with: self
	]

]
