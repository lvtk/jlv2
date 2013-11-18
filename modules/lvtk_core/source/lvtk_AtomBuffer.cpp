

AtomBuffer::AtomBuffer (uint32 bufferType, uint32 bufferSize)
    : type (bufferType), capacity (bufferSize)
{
    block.calloc (capacity);
    atom = (LV2_Atom*) block.getData();
}

AtomBuffer::~AtomBuffer()
{
    atom = nullptr;
    block.free();
}

bool
AtomBuffer::addEvent (int64 frames, uint32 size, uint32 type, const uint8* data)
{
	if (sizeof(LV2_Atom) + atom->size + lv2_atom_pad_size(size) > capacity) {
		return false;
	}
    
	LV2_Atom_Sequence* seq = (LV2_Atom_Sequence*) atom;
	LV2_Atom_Event*    ev  = (LV2_Atom_Event*) ((uint8*)seq + lv2_atom_total_size (&seq->atom));
    
	ev->time.frames = frames;
	ev->body.size   = size;
	ev->body.type   = type;
	memcpy (ev + 1, data, size);
    
	atom->size += sizeof (LV2_Atom_Event) + lv2_atom_pad_size (size);
	return true;
}

void
AtomBuffer::clear()
{
    if (isAudio() || isControl())
    {
        atom->size = capacity - sizeof (LV2_Atom);
	}
    else if (isSequence())
    {
		LV2_Atom_Sequence* seq = (LV2_Atom_Sequence*) atom;
		atom->type    = type;
		atom->size    = sizeof (LV2_Atom_Sequence_Body);
		seq->body.unit = 0;
		seq->body.pad  = 0;
	}
}
