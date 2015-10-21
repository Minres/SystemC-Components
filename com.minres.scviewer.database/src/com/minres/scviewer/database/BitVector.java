/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database;

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
	
	public String toHexString(){
		int resWidth=(width-1)/4+1;
		char[] res = new char[resWidth];
		for(int i=resWidth-1; i>=0; i--){
			int digit=0;
			for(int j=3; j>=0; j--){
				if(value[4*i+j]==VALUE_X ||value[4*i+j]==VALUE_Z ){
					res[i]=VALUE_X;
				}
				if(value[4*i+j]==VALUE_1)
					digit+=1<<(3-j);
				res[i]=Character.forDigit(digit, 16); //((digit < 10) ? '0' + digit : 'a' + digit -10)
			}
		}
		return new String(res);		
	}
}
