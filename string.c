#include "all.h"

fn bool stringsEq(String* a, String* b) {
  if (a->length != b->length) {
    return false;
  }
  for (u32 i = 0; i < a->length; i++) {
    if (a->bytes[i] != b->bytes[i]) {
      return false;
    }
  }
  return true;
}

fn bool cStringEqString(str a, String* b) {
  if (strlen(a) != b->length) {
    return false;
  }
  for (u32 i = 0; i < b->length; i++) {
    if (a[i] != b->bytes[i]) {
      return false;
    }
  }
  return true;
}

fn Utf8Character utf8CharacterClassify(u8 c) {
  /*two_byte utf8 starts with 1100. 192
  three_byte utf8 starts with 1110. 224
  four_byte utf8 starts with 1111. 240*/
  if (c <= 127) {
    return Utf8CharacterAscii;
  } else if (c >= 192 && c < 224) {
    return Utf8CharacterTwoByte;
  } else if (c >= 224 && c < 240) {
    return Utf8CharacterThreeByte;
  } else if (c >= 240) {
    return Utf8CharacterFourByte;
  } else {
    assert(false && "Not a valid utf8 starting byte");
    return Utf8Character_Count;
  }
}

fn Utf8Character utf8CharacterClassifyUnsafe(u8 c) {
  /*two_byte utf8 starts with 1100. 192
  three_byte utf8 starts with 1110. 224
  four_byte utf8 starts with 1111. 240*/
  if (c <= 127) {
    return Utf8CharacterAscii;
  } else if (c >= 192 && c < 224) {
    return Utf8CharacterTwoByte;
  } else if (c >= 224 && c < 240) {
    return Utf8CharacterThreeByte;
  } else if (c >= 240) {
    return Utf8CharacterFourByte;
  } else {
    return Utf8Character_Count;
  }
}

fn bool isUtf8Ascii(u8 c) {
  return classifyUtf8Character(c) == Utf8CharacterAscii;
}

fn bool isUtf8TwoByte(u8 c) {
  return classifyUtf8Character(c) == Utf8CharacterTwoByte;
}

fn bool isUtf8ThreeByte(u8 c) {
  return classifyUtf8Character(c) == Utf8CharacterThreeByte;
}

fn bool isUtf8FourByte(u8 c) {
  return classifyUtf8Character(c) == Utf8CharacterFourByte;
}

fn u8 lowerAscii(u8 c) {
  if (c >= 65 && c <= 90) {
    return c + 32;
  }
  return c;
}

fn u8 upperAscii(u8 c) {
  if (c >= 97 && c <= 122) {
    return c - 32;
  }
  return c;
}

fn bool isAlphaUnderscoreSpace(u8 c) {
  return ((c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z')
        || c == ' '
        || c == '_');
}

fn bool codepointIsWordBreak(Codepoint c) {
  return codepointIsWhitespace(c) ||
    c.code == ';' || c.code == ':' ||
    c.code == '.' || c.code == ',' || c.code == '!' || c.code == '?' ||
    c.code == '|' || c.code == '/' || c.code == '\\'
    ;
} 

fn bool codepointIsWhitespace(Codepoint c) {
  return c.code == ' ' || c.code == '\t' || c.code == '\n';
} 

fn Codepoint codepointFromBytes(ptr bytes, u32 offset) {
  Codepoint result = {0};
  result.type = utf8CharacterClassify(bytes[offset]);
  switch (result.type) {
    case Utf8CharacterAscii: {
      result.size = 1;
      result.code = bytes[offset];
    } break;
    case Utf8CharacterTwoByte: {
      result.size = 2;
      result.code = (
        bytes[offset] << 8 | bytes[offset+1]
      );
    } break;
    case Utf8CharacterThreeByte: {
      result.size = 3;
      result.code = (
        bytes[offset] << 16 | bytes[offset+1] << 8 | bytes[offset+2]
      );
    } break;
    case Utf8CharacterFourByte: {
      result.size = 4;
      result.code = (
        bytes[offset] << 24 | bytes[offset+1] << 16 | bytes[offset+2] << 8 | bytes[offset+3]
      );
    } break;
    case Utf8Character_Count: {
      printf("unabled to classify utf8 codepoint %d\n", bytes[offset]);
      assert(false);
    } break;
  }
  return result;
}

fn Codepoint codepointFromBytesBefore(ptr bytes, u32 offset) {
  assert(offset > 0);
  Codepoint result = { .type = Utf8Character_Count };
  u32 i = 0;
  while (result.type == Utf8Character_Count && i <= 4) {
    i++;
    offset -= 1;
    result.type = utf8CharacterClassifyUnsafe(bytes[offset]);
  }
  switch (result.type) {
    case Utf8CharacterAscii: {
      result.size = 1;
      result.code = bytes[offset];
    } break;
    case Utf8CharacterTwoByte: {
      result.size = 2;
      result.code = (
        bytes[offset] << 8 | bytes[offset+1]
      );
    } break;
    case Utf8CharacterThreeByte: {
      result.size = 3;
      result.code = (
        bytes[offset] << 16 | bytes[offset+1] << 8 | bytes[offset+2]
      );
    } break;
    case Utf8CharacterFourByte: {
      result.size = 4;
      result.code = (
        bytes[offset] << 24 | bytes[offset+1] << 16 | bytes[offset+2] << 8 | bytes[offset+3]
      );
    } break;
    case Utf8Character_Count: {
      printf("unabled to classify utf8 codepoint %d\n", bytes[offset]);
      assert(false);
    } break;
  }
  return result;
}

fn Codepoint codepointFromRawInt(u32 c) {
  Codepoint result = { .code = c };
  if (c <= 127) { // ascii
    result.type = Utf8CharacterAscii;
    result.size = 1;
  } else if ((c >> 8) >= 192 && (c >> 8) < 224) {
    result.type = Utf8CharacterTwoByte;
    result.size = 2;
  } else if ((c >> 16) >= 224 && (c >> 16) < 240) {
    result.type = Utf8CharacterThreeByte;
    result.size = 3;
  } else if ((c >> 24) >= 240) {
    result.type = Utf8CharacterFourByte;
    result.size = 4;
  }
  return result;
}

fn void codepointFillBuf(Codepoint cp, ptr buf) {
  switch (cp.type) {
    case Utf8CharacterAscii: {
      buf[0] = cp.code;
    } break;
    case Utf8CharacterTwoByte: {
      buf[0] = (cp.code & 0xFF00) >> 8;
      buf[1] = cp.code & 0xFF;
    } break;
    case Utf8CharacterThreeByte: {
      buf[0] = (cp.code & 0xFF0000) >> 16;
      buf[1] = (cp.code & 0xFF00) >> 8;
      buf[2] = cp.code & 0xFF;
    } break;
    case Utf8CharacterFourByte: {
      buf[0] = (cp.code & 0xFF000000) >> 24;
      buf[1] = (cp.code & 0xFF0000) >> 16;
      buf[2] = (cp.code & 0xFF00) >> 8;
      buf[3] = cp.code & 0xFF;
    } break;
    case Utf8Character_Count: {
      assert(false);
    } break;
  }
}

fn String stringFromRawCodepoint(Arena* a, u32 c) {
  String result = {0};
  Codepoint codepoint = codepointFromRawInt(c);
  switch (codepoint.type) {
    case Utf8CharacterAscii:      result.capacity = 1; break;
    case Utf8CharacterTwoByte:    result.capacity = 2; break;
    case Utf8CharacterThreeByte:  result.capacity = 3; break;
    case Utf8CharacterFourByte:   result.capacity = 4; break;
    case Utf8Character_Count: {
      assert(false);
    } break;
  }
  result.length = result.capacity;
  result.bytes = arenaAlloc(a, result.length);
  codepointFillBuf(codepoint, result.bytes);
  return result;
}

fn bool stringInsertCodepointAtByte(String* s, Codepoint c, u32 byte_offset) {
  u32 remaining_space = s->capacity - s->length;
  if (remaining_space < c.size) return false;

  char codepoint_bytes[4];
  codepointFillBuf(c, codepoint_bytes);
  // shift all the bytes over from the byte_offset onward
  for (u32 i = 0; i < c.size; i++) {
    for (u32 ii = s->length; ii > byte_offset+i; ii--) {
      s->bytes[ii] = s->bytes[ii-1];
    }
    s->length += 1;
    s->bytes[byte_offset+i] = codepoint_bytes[i];
  }
  return true;
}

fn bool stringDeleteCodepointAtByte(String* s, u32 byte_offset) {
  if (s->length < byte_offset) return false;

  Codepoint cp = codepointFromBytes(s->bytes, byte_offset);
  // shift all the bytes from the back towards the byte_offset
  for (u32 i = 0; i < cp.size; i++) {
    for (u32 ii = byte_offset+i; ii < s->length; ii++) {
      s->bytes[ii] = s->bytes[ii+1];
    }
    s->length -= 1;
  }
  return true;
}

fn bool isSimplePrintable(u8 c) {
  return (c >= ' ' && c <= '~');
}

typedef struct StrDecode {
	u32 codepoint;
	u32 size;
} StrDecode;

fn StrDecode strDecodeUTF8(u8 *string, u32 cap){
	u8 length[] = {
		1, 1, 1, 1, // 000xx
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
		0, 0, 0, 0, // 100xx
		0, 0, 0, 0,
		2, 2, 2, 2, // 110xx
		3, 3,       // 1110x
		4,          // 11110
		0,          // 11111
	};
	u8 first_byte_mask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
	u8 final_shift[] = { 0, 18, 12, 6, 0 };

	StrDecode result = {0};
	if (cap > 0){
		result.codepoint = '#';
		result.size = 1;

		u8 byte = string[0];
		u8 l = length[byte >> 3];
		if (0 < l && l <= cap){
			u32 cp = (byte & first_byte_mask[l]) << 18;
			switch (l){
				case 4: cp |= ((string[3] & 0x3F) << 0);
				case 3: cp |= ((string[2] & 0x3F) << 6);
				case 2: cp |= ((string[1] & 0x3F) << 12);
				default: break;
			}
			cp >>= final_shift[l];

			result.codepoint = cp;
			result.size = l;
		}
	}

	return result;
}

fn u32 strEncodeUTF8(u8 *dst, u32 codepoint){
	u32 size = 0;
	if (codepoint < (1 << 8)) {
		dst[0] = codepoint;
		size = 1;
	} else if (codepoint < (1 << 11)) {
		dst[0] = 0xC0 | (codepoint >> 6);
		dst[1] = 0x80 | (codepoint & 0x3F);
		size = 2;
	}
	else if (codepoint < (1 << 16)) {
		dst[0] = 0xE0 | (codepoint >> 12);
		dst[1] = 0x80 | ((codepoint >> 6) & 0x3F);
		dst[2] = 0x80 | (codepoint & 0x3F);
		size = 3;
	} else if (codepoint < (1 << 21)) {
		dst[0] = 0xF0 | (codepoint >> 18);
		dst[1] = 0x80 | ((codepoint >> 12) & 0x3F);
		dst[2] = 0x80 | ((codepoint >> 6) & 0x3F);
		dst[3] = 0x80 | (codepoint & 0x3F);
		size = 4;
	} else {
		dst[0] = '#';
		size = 1;
	}
	return size;
}

fn StrDecode strDecodeUTF16(u16 *str, u32 cap){
	StrDecode result = {'#', 1};
	u16 x = str[0];
	if (x < 0xD800 || 0xDFFF < x) {
		result.codepoint = x;
	} else if (cap >= 2) {
		u16 y = str[1];
		if (0xD800 <= x && x < 0xDC00 &&
			0xDC00 <= y && y < 0xE000
		) {
			u16 xj = x - 0xD800;
			u16 yj = y - 0xDc00;
			u32 xy = (xj << 10) | yj;
			result.codepoint = xy + 0x10000;
			result.size = 2;
		}
	}
	return result;
}

fn u32 strEncodeUTF16(u16 *dst, u32 codepoint){
	u32 size = 0;
	if (codepoint < 0x10000) {
		dst[0] = codepoint;
		size = 1;
	} else {
		u32 cpj = codepoint - 0x10000;
		dst[0] = (cpj >> 10) + 0xD800;
		dst[1] = (cpj & 0x3FF) + 0xDC00;
		size = 2;
	}
	return(size);
}

fn StringUTF16Const str16FromStr8(Arena* arena, String string) {
	u16* memory = arenaAllocArray(arena, u16, string.length * 2 + 1);

	u16* dptr = memory;
	u8* ptr = (u8*)string.bytes;
	u8* opl = (u8*)string.bytes + string.length;
	for (; ptr < opl;){
		StrDecode decode = strDecodeUTF8(ptr, (u64)(opl - ptr));
		u32 enc_size = strEncodeUTF16(dptr, decode.codepoint);
		ptr += decode.size;
		dptr += enc_size;
	}

	*dptr = 0;

	u64 alloc_count = string.length*2 + 1;
	u64 string_count = (u64)(dptr - memory);
	u64 unused_count = alloc_count - string_count - 1;
	arenaDealloc(arena, unused_count * sizeof(*memory));

	StringUTF16Const result = { memory, string_count };
	return result;
}

