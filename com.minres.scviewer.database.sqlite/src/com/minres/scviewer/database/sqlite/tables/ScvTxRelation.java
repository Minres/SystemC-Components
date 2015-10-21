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
package com.minres.scviewer.database.sqlite.tables;

public class ScvTxRelation {
	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public int getSrc() {
		return src;
	}

	public void setSrc(int src) {
		this.src = src;
	}

	public int getSink() {
		return sink;
	}

	public void setSink(int sink) {
		this.sink = sink;
	}

	private String name;
	private int src;
	private int sink;

}