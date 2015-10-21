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

public class ScvGenerator {
	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getStream() {
		return stream;
	}

	public void setStream(int stream) {
		this.stream = stream;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public int getBegin_attr() {
		return begin_attr;
	}

	public void setBegin_attr(int begin_attr) {
		this.begin_attr = begin_attr;
	}

	public int getEnd_attr() {
		return end_attr;
	}

	public void setEnd_attr(int end_attr) {
		this.end_attr = end_attr;
	}

	private int id;
	private int stream;
	private String name;
	private int begin_attr;
	private int end_attr;
}