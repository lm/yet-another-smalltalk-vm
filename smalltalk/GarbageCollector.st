Object subclass: GarbageCollector [

	class collectGarbage [
		<primitive: GCPrimitive>
	]


	class printHeap [
		<primitive: PrintHeapPrimitive>
	]


	class lastStats [
		<primitive: LastGCStatsPrimitive>
	]

]
