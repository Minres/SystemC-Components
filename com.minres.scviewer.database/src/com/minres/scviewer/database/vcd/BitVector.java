package com.minres.scviewer.database.vcd;

public class BitVector {

	public static final char VALUE_X = 'X';
	public static final char VALUE_Z = 'Z';
	public static final char VALUE_1 = '1';
	public static final char VALUE_0 = '0';

	private final int width;
	
	private char[] value;
	
	public BitVector(int netWidth) {
		this.width=netWidth;
		value = new char[netWidth];
		for(int i=0; i<netWidth; i++) value[i]='0';
	}

	public void setValue(int i, char value) {
		this.value[i]=value;
	}

	public char[] getValue() {
		return value;
	}

	public void setValue(char[] value) {
		this.value = value;
	}

	public int getWidth() {
		return width;
	}

	public String toString(){
		return new String(value);
	}
}
