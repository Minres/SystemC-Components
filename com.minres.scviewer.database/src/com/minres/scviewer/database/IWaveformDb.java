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

import java.io.File;
import java.util.List;


public interface IWaveformDb extends IHierNode {

	public Long getMaxTime();
	
	public IWaveform<? extends IWaveformEvent> getStreamByName(String name);
	
	public List<IWaveform<?>> getAllWaves();
	
	public List<RelationType> getAllRelationTypes();
	
	public boolean load(File inp) throws Exception;

	public boolean isLoaded();

	public void clear();

}
