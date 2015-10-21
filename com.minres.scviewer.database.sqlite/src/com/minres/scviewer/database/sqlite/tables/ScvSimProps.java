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

public class ScvSimProps {
	
	private long time_resolution;

	public ScvSimProps() {
		super();
	}

	public long getTime_resolution() {
		return time_resolution;
	}

	public void setTime_resolution(long time_resolution) {
		this.time_resolution = time_resolution;
	}

}
