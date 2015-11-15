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
import java.util.Collection;
import java.util.List;

public interface IWaveformDbLoader {
	
	public boolean load(IWaveformDb db, File inp) throws Exception;
	
	public Long getMaxTime();
	
	public List<IWaveform<? extends IWaveformEvent>> getAllWaves() ;
	
	public Collection<RelationType> getAllRelationTypes() ;
	
}
