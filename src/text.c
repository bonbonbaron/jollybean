/*
Things the scene will need:
	huffman tree
    offset + len of scene's concatenated, encoded text
    huffman decode it
    each sprite points at a chunk of the text and "speaks" it. That means null terminators must exist in substrings; this way you don't have to know the length.
    So in order to encode something like this, you're going to need to do the following:
		write the scene's script
	auto-assignment of strings with clear names
	auto 
*/