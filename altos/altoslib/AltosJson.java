/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.util.*;
import java.text.*;
import java.lang.*;
import java.lang.reflect.*;

class JsonUtil {
	Writer quote(Writer writer, String a) throws IOException {
		writer.append("\"");
		for (int i = 0; i < a.length(); i++) {
			char c = a.charAt(i);

			switch (c) {
			case '"':
			case '\\':
				writer.append('\\').append(c);
				break;
			case '\n':
				writer.append("\\n");
				break;
			default:
				writer.append(c);
				break;
			}
		}
		writer.append("\"");
		return writer;
	}

	Writer append(Writer result, AltosJson value, int indent, boolean pretty) throws IOException {
		value.append(result, indent, pretty);
		return result;
	}

	Writer append(Writer result, String string) throws IOException {
		result.append(string);
		return result;
	}

	Writer indent(Writer result, int indent) throws IOException {
		result.append("\n");
		for (int i = 0; i < indent; i++)
			result.append("\t");
		return result;
	}

	NumberFormat _nf_json;

	NumberFormat nf_json() {
		if (_nf_json == null) {
			DecimalFormat nf = (DecimalFormat) NumberFormat.getNumberInstance(Locale.ROOT);
			nf.setParseIntegerOnly(false);
			nf.setGroupingUsed(false);
			nf.setMaximumFractionDigits(17);
			nf.setMinimumFractionDigits(0);
			nf.setMinimumIntegerDigits(1);
			nf.setDecimalSeparatorAlwaysShown(false);
			_nf_json = nf;
		}
		return _nf_json;
	}
}

class JsonHash extends JsonUtil {
	Hashtable<String,AltosJson> hash;

	void append_hash(Writer result, int indent, boolean pretty) throws IOException {
		boolean		first = true;

		result.append("{");

		ArrayList<String> key_list = new ArrayList<String>(hash.keySet());

		Collections.sort(key_list, new Comparator<String>() {
				@Override
				public int compare(String a, String b) { return a.compareTo(b); }
			});

		for (String key : key_list) {
			AltosJson	value = hash.get(key);

			if (!first)
				result.append(",");
			first = false;
			if (pretty)
				indent(result, indent+1);
			quote(result, key);
			append(result, ": ");
			append(result, value, indent+1, pretty);
		}
		if (pretty)
			indent(result, indent);
		append(result, "}");
	}

	void put(String key, AltosJson value) {
		hash.put(key, value);
	}

	AltosJson get(String key) {
		return hash.get(key);
	}

	JsonHash() {
		hash = new Hashtable<String,AltosJson>();
	}
}

class JsonArray extends JsonUtil {
	ArrayList<AltosJson> array;

	void append_array(Writer result, int indent, boolean pretty) throws IOException {
		boolean first = true;

		append(result, "[");
		for (int i = 0; i < array.size(); i++) {
			AltosJson	value = array.get(i);

			if (!first)
				append(result, ",");
			first = false;
			if (pretty)
				indent(result, indent+1);
			append(result, value, indent+1, pretty);
		}
		if (pretty)
			indent(result, indent);
		append(result, "]");
	}

	void put(int index, AltosJson value) {
		if (index >= array.size())
			array.add(index, value);
		else
			array.set(index, value);
	}

	AltosJson get(int index) {
		if (index < 0 || index > array.size())
			return null;
		return array.get(index);
	}

	int size() {
		return array.size();
	}

	JsonArray() {
		array = new ArrayList<AltosJson>();
	}
}

class JsonToken {
	double	dval;
	long	lval;
	String	sval;
	boolean	bval;
	int	token;

	static final int _string = 0;
	static final int _double = 1;
	static final int _long = 2;
	static final int _boolean = 3;
	static final int _oc = 4;
	static final int _cc = 5;
	static final int _os = 6;
	static final int _cs = 7;
	static final int _comma = 8;
	static final int _colon = 9;
	static final int _end = 10;
	static final int _error = 11;
	static final int _none = 12;

	static String token_name(int token) {
		switch (token) {
		case _string:
			return "string";
		case _double:
			return "number";
		case _long:
			return "number";
		case _boolean:
			return "boolean";
		case _oc:
			return "{";
		case _cc:
			return "}";
		case _os:
			return "[";
		case _cs:
			return "]";
		case _comma:
			return ",";
		case _colon:
			return ":";
		case _end:
			return "<EOF>";
		case _error:
			return "<ERROR>";
		default:
			return "<UNKNOWN>";
		}
	}

	String token_name() {
		return token_name(token);
	}

	JsonToken(int token) {
		this.token = token;
	}

	JsonToken(int token, boolean bval) {
		this.token = token;
		this.bval = bval;
	}

	JsonToken(int token, double dval) {
		this.token = token;
		this.dval = dval;
	}

	JsonToken(int token, long lval) {
		this.token = token;
		this.lval = lval;
	}

	JsonToken(int token, String sval) {
		this.token = token;
		this.sval = sval;
	}

	JsonToken(int token, Writer bval) {
		this(token, bval.toString());
	}
}

/*
 * Lexer for json
 */
class JsonLexer extends JsonUtil {
	InputStream		f;
	int			line;
	int			ungot = -2;
	StringBuffer		pending_token;
	private JsonToken	token;

	static class keyword {
		String		word;
		JsonToken	token;

		JsonToken match(String value) {
			if (word.equals(value))
				return token;
			return null;
		}

		keyword(String word, JsonToken token) {
			this.word = word;
			this.token = token;
		}
	}

	/* boolean values are the only keywords in json
	 */
	static keyword[] keywords = {
		new keyword("true", new JsonToken(JsonToken._boolean, true)),
		new keyword("false", new JsonToken(JsonToken._boolean, false)),
		new keyword("NegInfinity", new JsonToken(JsonToken._double, Double.NEGATIVE_INFINITY)),
		new keyword("Infinity", new JsonToken(JsonToken._double, Double.POSITIVE_INFINITY)),
		new keyword("NaN", new JsonToken(JsonToken._double, Double.NaN))
	};

	static JsonToken keyword(String word) {
		for (int i = 0; i < keywords.length; i++) {
			JsonToken token = keywords[i].match(word);
			if (token != null)
				return token;
		}
		return null;
	}

	/* Get the next char (-1 for EOF) */
	int ch() throws IOException {
		int c;
		if (ungot != -2) {
			c = ungot;
			ungot = -2;
		} else
			c = f.read();
		if (c != -1)
			pending_token.append((char) c);
		if (c == '\n')
			++line;
		return c;
	}

	void unch(int c) {
		if (ungot != -2)
			throw new IllegalArgumentException("ungot buffer full");
		pending_token.deleteCharAt( pending_token.length()-1);
		if (c == '\n')
			--line;
		ungot = c;
	}

	String last_token_string() {
		if (pending_token == null)
			return null;

		return pending_token.toString();
	}

	static boolean is_long_range(double d) {
		return -9223372036854775808.0 <= d && d <= 9223372036854775807.0;
	}

	JsonToken lex() {
		pending_token = new StringBuffer();

		try {
			for (;;) {
				int c = ch();

				switch (c) {
				case -1:
					return new JsonToken(JsonToken._end);
				case '\n':
				case ' ':
				case '\t':
					continue;
				case '{':
					return new JsonToken(JsonToken._oc);
				case '}':
					return new JsonToken(JsonToken._cc);
				case '[':
					return new JsonToken(JsonToken._os);
				case ']':
					return new JsonToken(JsonToken._cs);
				case ',':
					return new JsonToken(JsonToken._comma);
				case ':':
					return new JsonToken(JsonToken._colon);
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
				case '.': case '-': case '+':
					StringBuffer dbuf = new StringBuffer();
					boolean is_double = false;
					while (Character.isDigit(c) || c == '.' || c == '+' || c == '-' || c == 'e' || c == 'E') {
						if (c == '.' || c == 'E')
							is_double = true;
						dbuf.appendCodePoint(c);
						c = ch();
					}
					unch(c);
					String dstr = dbuf.toString();
					double dval;
					try {
						dval = nf_json().parse(dstr).doubleValue();
					} catch (ParseException pe) {
						return new JsonToken(JsonToken._error, dstr);
					}
					if (is_double || !is_long_range(dval))
						return new JsonToken(JsonToken._double, dval);
					else {
						long lval = Long.parseLong(dstr);
						return new JsonToken(JsonToken._long, lval);
					}
				case '"':
					Writer bval = new StringWriter();
					for (;;) {
						c = ch();
						if (c == '"')
							break;
			 			if (c == '\\') {
							c = ch();
							switch (c) {
							case 'n':
								c = '\n';
								break;
							case 't':
								c = '\t';
								break;
							default:
								break;
							}
						}
						bval.write(c);
					}
					return new JsonToken(JsonToken._string, bval);
				default:
					if (Character.isLetter(c)) {
						StringBuffer tbuf = new StringBuffer();
						do {
							tbuf.appendCodePoint(c);
							c = ch();
						} while (Character.isLetter(c));
						unch(c);
						JsonToken token = keyword(tbuf.toString());
						if (token != null)
							return token;
					}
					break;
				}
			}
		} catch (IOException ie) {
			return new JsonToken(JsonToken._error, "<EIO>");
		}
	}

	void next() {
		token = null;
	}

	JsonToken token() {
		if (token == null)
			token = lex();
		return token;
	}

	JsonToken expect(int e) {
		JsonToken t = token();
		if (t.token != e)
			throw new IllegalArgumentException(String.format("got \"%s\" while expecting \"%s\"",
									 token.token_name(),
									 JsonToken.token_name(e)));
		next();
		return t;
	}

	JsonLexer(String s) {
		f = new AltosStringInputStream(s);
		line = 1;
		token = null;
	}

	JsonLexer(InputStream f) {
		this.f = f;
		line = 1;
		token = null;
	}
}

/*
 * Parse a json string into a AltosJson object
 */
class JsonParse {
	JsonLexer	lexer;

	void parse_error(String format, Object ... arguments) {
		throw new IllegalArgumentException(String.format("line %d: JSON parse error %s\n",
								 lexer.line,
								 String.format(format, arguments)));
	}

	/* Hashes are { string: value ... } */
	JsonHash hash() {
		JsonHash	hash = new JsonHash();

		/* skip the open brace */
		lexer.next();
		for (;;) {
			/* Allow for empty hashes */
			if (lexer.token().token == JsonToken._cc) {
				lexer.next();
				return hash;
			}

			/* string : value */
			String key = lexer.expect(JsonToken._string).sval;
			lexer.expect(JsonToken._colon);
			AltosJson value = value();
			hash.put(key, value);

			switch (lexer.token().token) {
			case JsonToken._comma:
				lexer.next();
				break;
			case JsonToken._cc:
				lexer.next();
				return hash;
			default:
				parse_error("got %s expect \",\" or \"}\"", lexer.token().token_name());
				return null;
			}
		}
	}

	/* Arrays are [ value ... ] */
	JsonArray array() {
		JsonArray	array = new JsonArray();

		lexer.next();
		for (int i = 0;; i++) {
			/* Allow for empty arrays */
			if (lexer.token().token == JsonToken._cs) {
				lexer.next();
				return array;
			}

			AltosJson value = value();
			array.put(i, value);
			switch (lexer.token().token) {
			case JsonToken._comma:
				lexer.next();
				break;
			case JsonToken._cs:
				lexer.next();
				return array;
			default:
				parse_error("got %s expect \",\" or \"]\"", lexer.token().token_name());
				return null;
			}
		}
	}

	/* Json is a simple LL language; one token is sufficient to
	 * identify the next object in the input
	 */
	AltosJson value() {
		switch (lexer.token().token) {
		case JsonToken._oc:
			return new AltosJson(hash());
		case JsonToken._os:
			return new AltosJson(array());
		case JsonToken._double:
			double dval = lexer.token().dval;
			lexer.next();
			return new AltosJson(dval);
		case JsonToken._long:
			long lval = lexer.token().lval;
			lexer.next();
			return new AltosJson(lval);
		case JsonToken._string:
			String sval = lexer.token().sval;
			lexer.next();
			return new AltosJson(sval);
		case JsonToken._boolean:
			boolean bval = lexer.token().bval;
			lexer.next();
			return new AltosJson(bval);
		default:
			parse_error("Unexpected token \"%s\"", lexer.token().token_name());
		}
		return null;
	}

	AltosJson parse() {
		lexer.next();
		return value();
	}

	JsonParse(String s) {
		lexer = new JsonLexer(s);
	}

	JsonParse(InputStream f) {
		lexer = new JsonLexer(f);
	}
}

public class AltosJson extends JsonUtil {
	private static final int	type_none = 0;
	private static final int	type_hash = 1;
	private static final int	type_array = 2;
	private static final int	type_double = 3;
	private static final int	type_long = 4;
	private static final int	type_string = 5;
	private static final int	type_boolean = 6;

	private int		type;

	private JsonHash	hash;
	private JsonArray	array;
	private double		d_number;
	private long		l_number;
	private String		string;
	private boolean		bool;

	/* Generate string representation of the value
	 */
	Writer append(Writer result, int indent, boolean pretty) throws IOException {
		switch (type) {
		case type_hash:
			hash.append_hash(result, indent, pretty);
			break;
		case type_array:
			array.append_array(result, indent, pretty);
			break;
		case type_double:
			if (Double.isInfinite(d_number)) {
				if (d_number < 0)
					result.append("NegInfinity");
				else
					result.append("Infinity");
			} else if (Double.isNaN(d_number)) {
				result.append("NaN");
			} else {
				String dval = nf_json().format(d_number);
				if (dval.equals("-0"))
					dval = "0";
				result.append(dval);
			}
			break;
		case type_long:
			result.append(Long.valueOf(l_number).toString());
			break;
		case type_string:
			quote(result, string);
			break;
		case type_boolean:
			result.append(bool ? "true" : "false");
			break;
		}
		return result;
	}

	private String toString(int indent, boolean pretty) {
		try {
			Writer result = new StringWriter();
			append(result, indent, pretty);
			return result.toString();
		} catch (Exception e) {
			return null;
		}
	}

	public String toString() {
		return toString(0, false);
	}

	public String toPrettyString() {
		return toString(0, true);
	}

	public void write(Writer w, int indent, boolean pretty) throws IOException {
		append(w, indent, pretty);
	}

	public void write(Writer w) throws IOException {
		write(w, 0, true);
	}

	/* Parse string representation to a value
	 */

	public static AltosJson fromString(String string) {
		JsonParse	parse = new JsonParse(string);
		try {
			return parse.parse();
		} catch (IllegalArgumentException ie) {
			System.out.printf("json:\n%s\n%s\n", string, ie.getMessage());
			return null;
		}
	}

	public static AltosJson fromInputStream(InputStream f) {
		JsonParse	parse = new JsonParse(f);
		try {
			return parse.parse();
		} catch (IllegalArgumentException ie) {
			System.out.printf("json:\n%s\n", ie.getMessage());
			return null;
		}
	}

	/* Accessor functions
	 */
	private boolean assert_type(boolean setting, int type, int other_type, String error) {
		if (setting && this.type == type_none) {
			this.type = type;
			return false;
		}
		if (this.type != type && this.type != other_type)
			throw new IllegalArgumentException(error);
		return true;
	}

	private boolean assert_type(boolean setting, int type, String error) {
		return assert_type(setting, type, type, error);
	}

	private void assert_hash(boolean setting) {
		if (!assert_type(setting, type_hash, "not a hash"))
			hash = new JsonHash();
	}

	private void assert_array(boolean setting) {
		if (!assert_type(setting, type_array, "not an array"))
			array = new JsonArray();
	}

	private void assert_number() {
		assert_type(false, type_double, type_long, "not a number");
	}

	private void assert_double() {
		assert_type(true, type_double, type_long, "not a number");
	}

	private void assert_long() {
		assert_type(true, type_long, type_double, "not a number");
	}

	private void assert_string(boolean setting) {
		assert_type(setting, type_string, "not a string");
	}

	private void assert_boolean(boolean setting) {
		assert_type(setting, type_boolean, "not a boolean");
	}

	/* Primitive accessors
	 */
	public double number() {
		assert_number();
		if (type == type_double)
			return d_number;
		else
			return (double) l_number;
	}

	public long l_number() {
		assert_number();
		if (type == type_double)
			return (long) d_number;
		else
			return l_number;
	}

	public String string() {
		assert_string(false);
		return string;
	}

	public boolean bool() {
		assert_boolean(false);
		return bool;
	}

	public AltosJson get(int index) {
		assert_array(false);
		return array.get(index);
	}

	public AltosJson get(String key) {
		assert_hash(false);
		return hash.get(key);
	}

	public int size() {
		assert_array(false);
		return array.size();
	}

	/* Typed accessors with defaulting
	 */
	public double get_double(String key, double def) {
		AltosJson value = get(key);
		if (value != null) {
			return value.number();
		}
		return def;
	}

	public long get_long(String key, long def) {
		AltosJson value = get(key);
		if (value != null)
			return value.l_number();
		return def;
	}

	public int get_int(String key, int def) {
		AltosJson value = get(key);
		if (value != null)
			return (int) value.l_number();
		return def;
	}

	public String get_string(String key, String def) {
		AltosJson value = get(key);
		if (value != null)
			return value.string();
		return def;
	}

	public boolean get_boolean(String key, boolean def) {
		AltosJson value = get(key);
		if (value != null)
			return value.bool();
		return def;
	}

	public double get_double(int index, double def) {
		AltosJson value = get(index);
		if (value != null)
			return value.number();
		return def;
	}

	public long get_long(int index, long def) {
		AltosJson value = get(index);
		if (value != null)
			return value.l_number();
		return def;
	}

	public int get_int(int index, int def) {
		AltosJson value = get(index);
		if (value != null)
			return (int) value.l_number();
		return def;
	}

	public String get_string(int index, String def) {
		AltosJson value = get(index);
		if (value != null)
			return value.string();
		return def;
	}

	public boolean get_boolean(int index, boolean def) {
		AltosJson value = get(index);
		if (value != null)
			return value.bool();
		return def;
	}

	public double[] get_double_array(String key, double[] def) {
		AltosJson value = get(key);
		if (value != null) {
			double[] ret = new double[value.size()];
			for (int i = 0; i < value.size(); i++)
				ret[i] = value.get_double(i, def == null ? 0 : def[i]);
			return ret;
		}
		return def;
	}

	public int[] get_int_array(String key, int[] def) {
		AltosJson value = get(key);
		if (value != null) {
			int[] ret = new int[value.size()];
			for (int i = 0; i < value.size(); i++)
				ret[i] = value.get_int(i, def == null ? 0 : def[i]);
			return ret;
		}
		return def;
	}

	/* Array setter functions
	 */
	public AltosJson put(int index, AltosJson value) {
		assert_array(true);
		array.put(index, value);
		return value;
	}

	public Object put(int index, Object value) {
		assert_array(true);
		if (value != null)
			array.put(index, new AltosJson(value));
		return value;
	}

	public double put(int index, double value) {
		assert_array(true);
		array.put(index, new AltosJson(value));
		return value;
	}

	public AltosJson put(int index, double[] value) {
		if (value != null) {
			assert_array(true);
			array.put(index, new AltosJson(value));
		}
		return this;
	}

	public int[] put(int index, int[] value) {
		if (value != null) {
			assert_array(true);
			array.put(index, new AltosJson(value));
		}
		return value;
	}

	public String put(int index, String value) {
		if (value != null) {
			assert_array(true);
			array.put(index, new AltosJson(value));
		}
		return value;
	}

	public boolean put(int index, boolean value) {
		assert_array(true);
		array.put(index, new AltosJson(value));
		return value;
	}

	/* Hash setter functions
	 */
	public AltosJson put(String key, AltosJson value) {
		assert_hash(true);
		hash.put(key, value);
		return value;
	}

	public Object put(String key, Object value) {
		assert_hash(true);
		if (value != null)
			hash.put(key, new AltosJson(value));
		return value;
	}

	public double put(String key, double value) {
		assert_hash(true);
		hash.put(key, new AltosJson(value));
		return value;
	}

	public String put(String key, String value) {
		if (value != null) {
			assert_hash(true);
			hash.put(key, new AltosJson(value));
		}
		return value;
	}

	public boolean put(String key, boolean value) {
		assert_hash(true);
		hash.put(key, new AltosJson(value));
		return value;
	}

	public AltosJson[] put(String key, AltosJson[] value) {
		if (value != null) {
			assert_hash(true);
			hash.put(key, new AltosJson(value));
		}
		return value;
	}

	public double[] put(String key, double[] value) {
		if (value != null) {
			assert_hash(true);
			hash.put(key, new AltosJson(value));
		}
		return value;
	}

	public int[] put(String key, int[] value) {
		if (value != null) {
			assert_hash(true);
			hash.put(key, new AltosJson(value));
		}
		return value;
	}

	/* Primitive setter functions
	 */
	public double put(double value) {
		assert_double();
		d_number = value;
		return value;
	}

	public byte put(byte value) {
		assert_long();
		l_number = value;
		return value;
	}

	public char put(char value) {
		assert_long();
		l_number = value;
		return value;
	}

	public int put(int value) {
		assert_long();
		l_number = value;
		return value;
	}

	public long put(long value) {
		assert_long();
		l_number = value;
		return value;
	}

	public String put(String value) {
		assert_string(true);
		string = value;
		return value;
	}

	public boolean put(boolean value) {
		assert_boolean(true);
		bool = value;
		return value;
	}

	private boolean isInnerClass(Class c) {
		for (Field field : c.getDeclaredFields())
			if (field.isSynthetic())
				return true;
		return false;
	}

	/* Construct an object of the specified class from the JSON
	 * representation.
	 *
	 * This works as long as the structure is non-recursive, and
	 * all inner classes are only members of their immediate outer
	 * class
	 */
	@SuppressWarnings("unchecked")
	private Object make(Class c, Class enclosing_class, Object enclosing_object) {
		Object	ret;
		if (c == Boolean.TYPE) {
			ret = bool();
		} else if (c == Byte.TYPE) {
			ret = (Byte) (byte) l_number();
		} else if (c == Character.TYPE) {
			ret = (Character) (char) l_number();
		} else if (c == Integer.TYPE) {
			ret = (Integer) (int) l_number();
		} else if (c == Long.TYPE) {
			ret = l_number();
		} else if (c == Double.TYPE) {
			ret = number();
		} else if (c == String.class) {
			ret = string();
		} else if (c.isArray()) {
			assert_array(false);

			Class element_class = c.getComponentType();
			if (element_class == Boolean.TYPE) {
				boolean[] array = (boolean[]) Array.newInstance(element_class, size());
				for (int i = 0; i < array.length; i++)
					array[i] = (Boolean) get(i).make(element_class);
				ret = array;
			} else if (element_class == Byte.TYPE) {
				byte[] array = (byte[]) Array.newInstance(element_class, size());
				for (int i = 0; i < array.length; i++)
					array[i] = (Byte) get(i).make(element_class);
				ret = array;
			} else if (element_class == Character.TYPE) {
				char[] array = (char[]) Array.newInstance(element_class, size());
				for (int i = 0; i < array.length; i++)
					array[i] = (Character) get(i).make(element_class);
				ret = array;
			} else if (element_class == Integer.TYPE) {
				int[] array = (int[]) Array.newInstance(element_class, size());
				for (int i = 0; i < array.length; i++)
					array[i] = (Integer) get(i).make(element_class);
				ret = array;
			} else if (element_class == Long.TYPE) {
				long[] array = (long[]) Array.newInstance(element_class, size());
				for (int i = 0; i < array.length; i++)
					array[i] = (Long) get(i).make(element_class);
				ret = array;
			} else if (element_class == Double.TYPE) {
				double[] array = (double[]) Array.newInstance(element_class, size());
				for (int i = 0; i < array.length; i++)
					array[i] = (Double) get(i).make(element_class);
				ret = array;
			} else {
				Object[] array = (Object[]) Array.newInstance(element_class, size());
				for (int i = 0; i < array.length; i++)
					array[i] = get(i).make(element_class);
				ret = array;
			}
		} else {
			assert_hash(false);
			Object object = null;
			try {
				/* Inner classes have a hidden extra parameter
				 * to the constructor. Assume that the enclosing object is
				 * of the enclosing class and construct the object
				 * based on that.
				 */
				if (enclosing_class != null && isInnerClass(c)) {
					Constructor<?> ctor = ((Class<?>)c).getDeclaredConstructor((Class<?>) enclosing_class);
					object = ctor.newInstance(enclosing_object);
				} else {
					object = c.getDeclaredConstructor().newInstance();
				}
				for (; c != Object.class; c = c.getSuperclass()) {
					for (Field field : c.getDeclaredFields()) {
						String	fieldName = field.getName();
						Class	fieldClass = field.getType();

						if (Modifier.isStatic(field.getModifiers()))
							continue;
						if (field.isSynthetic())
							continue;
						try {
							AltosJson json = get(fieldName);
							if (json != null) {
								Object val = json.make(fieldClass, c, object);
								field.setAccessible(true);
								field.set(object, val);
							}
						} catch (IllegalAccessException ie) {
							System.out.printf("%s:%s %s\n",
									  c.getName(), fieldName, ie.toString());
						}
					}
				}
				ret = object;
			} catch (InvocationTargetException ie) {
				System.out.printf("%s: %s\n",
						  c.getName(), ie.toString());
				ret = null;
			} catch (NoSuchMethodException ie) {
				System.out.printf("%s: %s\n",
						  c.getName(), ie.toString());
				ret = null;
			} catch (InstantiationException ie) {
				System.out.printf("%s: %s\n",
						  c.getName(), ie.toString());
				ret = null;
			} catch (IllegalAccessException ie) {
				System.out.printf("%s: %s\n",
						  c.getName(), ie.toString());
				ret = null;
			}
		}
		return ret;
	}

	/* This is the public API for the
	 * above function which doesn't handle
	 * inner classes
	 */
	public Object make(Class c) {
		return make(c, null, null);
	}

	/* Constructors, one for each primitive type, String and Object */
	public AltosJson(boolean bool) {
		type = type_boolean;
		this.bool = bool;
	}

	public AltosJson(byte number) {
		type = type_long;
		this.l_number = number;
	}

	public AltosJson(char number) {
		type = type_long;
		this.l_number = number;
	}

	public AltosJson(int number) {
		type = type_long;
		this.l_number = number;
	}

	public AltosJson(long number) {
		type = type_long;
		this.l_number = number;
	}

	public AltosJson(double number) {
		type = type_double;
		this.d_number = number;
	}

	public AltosJson(String string) {
		type = type_string;
		this.string = string;
	}

	public AltosJson(Object object) {
		if (object instanceof Boolean) {
			type = type_boolean;
			bool = (Boolean) object;
		} else if (object instanceof Byte) {
			type = type_long;
			l_number = (Byte) object;
		} else if (object instanceof Character) {
			type = type_long;
			l_number = (Character) object;
		} else if (object instanceof Integer) {
			type = type_long;
			l_number = (Integer) object;
		} else if (object instanceof Long) {
			type = type_long;
			l_number = (Long) object;
		} else if (object instanceof Double) {
			type = type_double;
			d_number = (Double) object;
		} else if (object instanceof String) {
			type = type_string;
			string = (String) object;
		} else if (object == null) {
			System.out.printf("unexpected null object\n");
		} else if (object.getClass() == null) {
			System.out.printf("unexpected null object class\n");
		} else if (object.getClass().isArray()) {
			assert_array(true);

			Class component_class = object.getClass().getComponentType();
			if (component_class == Boolean.TYPE) {
				boolean[] array = (boolean[]) object;
				for (int i = 0; i < array.length; i++)
					put(i, new AltosJson(array[i]));
			} else if (component_class == Byte.TYPE) {
				byte[] array = (byte[]) object;
				for (int i = 0; i < array.length; i++)
					put(i, new AltosJson(array[i]));
			} else if (component_class == Character.TYPE) {
				char[] array = (char[]) object;
				for (int i = 0; i < array.length; i++)
					put(i, new AltosJson(array[i]));
			} else if (component_class == Integer.TYPE) {
				int[] array = (int[]) object;
				for (int i = 0; i < array.length; i++)
					put(i, new AltosJson(array[i]));
			} else if (component_class == Long.TYPE) {
				long[] array = (long[]) object;
				for (int i = 0; i < array.length; i++)
					put(i, new AltosJson(array[i]));
			} else if (component_class == Double.TYPE) {
				double[] array = (double[]) object;
				for (int i = 0; i < array.length; i++)
					put(i, new AltosJson(array[i]));
			} else {
				Object[] array = (Object[]) object;
				for (int i = 0; i < array.length; i++)
					put(i, new AltosJson(array[i]));
			}
		} else {
			assert_hash(true);
			for (Class c = object.getClass(); c != Object.class; c = c.getSuperclass()) {
				for (Field field : c.getDeclaredFields()) {
					String	fieldName = field.getName();

					/* XXX hack to allow fields to be not converted */
					if (fieldName.startsWith("__"))
						continue;

					/* Skip static fields */
					if (Modifier.isStatic(field.getModifiers()))
						continue;

					/* Skip synthetic fields. We're assuming
					 * those are always an inner class reference
					 * to the outer class object
					 */
					if (field.isSynthetic())
						continue;
					try {
						/* We may need to force the field to be accessible if
						 * it is private
						 */
						field.setAccessible(true);
						Object val = field.get(object);
						if (val != null) {
							AltosJson json = new AltosJson(val);
							put(fieldName, json);
						}
					} catch (IllegalAccessException ie) {
						System.out.printf("%s:%s %s\n",
								  c.getName(), fieldName, ie.toString());
					}
				}
			}
		}
	}

	/* Array constructors, one for each primitive type, String and Object */
	public AltosJson(boolean[] bools) {
		assert_array(true);
		for(int i = 0; i < bools.length; i++)
			put(i, new AltosJson(bools[i]));
	}

	public AltosJson(byte[] numbers) {
		assert_array(true);
		for(int i = 0; i < numbers.length; i++)
			put(i, new AltosJson(numbers[i]));
	}

	public AltosJson(char[] numbers) {
		assert_array(true);
		for(int i = 0; i < numbers.length; i++)
			put(i, new AltosJson(numbers[i]));
	}

	public AltosJson(int[] numbers) {
		assert_array(true);
		for(int i = 0; i < numbers.length; i++)
			put(i, new AltosJson(numbers[i]));
	}

	public AltosJson(long[] numbers) {
		assert_array(true);
		for(int i = 0; i < numbers.length; i++)
			put(i, new AltosJson(numbers[i]));
	}

	public AltosJson(double[] numbers) {
		assert_array(true);
		for(int i = 0; i < numbers.length; i++)
			put(i, new AltosJson(numbers[i]));
	}

	public AltosJson(String[] strings) {
		assert_array(true);
		for(int i = 0; i < strings.length; i++)
			put(i, new AltosJson(strings[i]));
	}

	public AltosJson(AltosJson[] jsons) {
		assert_array(true);
		for (int i = 0; i < jsons.length; i++)
			put(i, jsons[i]);
	}

	public AltosJson(Object[] array) {
		assert_array(true);
		for (int i = 0; i < array.length; i++)
			put(i, new AltosJson(array[i]));
	}

	/* Empty constructor
	 */
	public AltosJson() {
		type = type_none;
	}

	public AltosJson(JsonHash hash) {
		type = type_hash;
		this.hash = hash;
	}

	public AltosJson(JsonArray array) {
		type = type_array;
		this.array = array;
	}
}
