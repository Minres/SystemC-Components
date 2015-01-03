/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database;

import java.io.File;
import java.io.InputStream;
import java.util.List;


public interface ITrDb extends ITrHierNode {

	public EventTime getMaxTime();
	
	public ITrStream getStreamByName(String name);
	
	public List<ITrStream> getAllStreams();
	
	public void load(File inp) throws InputFormatException;

	public void clear();

}
