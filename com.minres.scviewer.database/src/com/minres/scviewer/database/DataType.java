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

public enum DataType {
	BOOLEAN, // bool
	ENUMERATION, // enum
	INTEGER, // char, short, int, long, long long, sc_int, sc_bigint
	UNSIGNED, // unsigned { char, short, int, long, long long }, sc_uint,
				// sc_biguint
	FLOATING_POINT_NUMBER, // float, double
	BIT_VECTOR, // sc_bit, sc_bv
	LOGIC_VECTOR, // sc_logic, sc_lv
	FIXED_POINT_INTEGER, // sc_fixed
	UNSIGNED_FIXED_POINT_INTEGER, // sc_ufixed
	RECORD, // struct/class
	POINTER, // T*
	ARRAY, // T[N]
	STRING // string, std::string
};
