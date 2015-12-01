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
 
package com.minres.scviewer.e4.application.handlers;

import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.workbench.modeling.EPartService;

import com.minres.scviewer.e4.application.parts.WaveformViewer;

public class SetRelationTypeHandler {
	final static String PARAMTER_ID="com.minres.scviewer.e4.application.commandparameter.relationName";

	@Execute
	public void execute(@Named(PARAMTER_ID) String relationName, EPartService partService) {
		MPart part = partService.getActivePart();
		Object obj = part.getObject();
		if(obj instanceof WaveformViewer){
			WaveformViewer waveformViewerPart = (WaveformViewer) obj;
			waveformViewerPart.setNavigationRelationType(relationName);
		}
	}
		
}