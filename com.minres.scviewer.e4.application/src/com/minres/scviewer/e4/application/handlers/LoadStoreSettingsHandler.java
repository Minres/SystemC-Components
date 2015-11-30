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

import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.ui.model.application.MApplication;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Shell;

import com.minres.scviewer.e4.application.parts.WaveformViewer;
public class LoadStoreSettingsHandler {

	static final String PARAMETER_ID="com.minres.scviewer.e4.application.commandparameter.loadStore";

	@CanExecute
	public boolean canExecute(EPartService partService) {
		MPart part = partService.getActivePart();
		if(part==null) return false;
		return (part.getObject() instanceof WaveformViewer);
	}

	@Execute
	public void execute(@Named(PARAMETER_ID) String param, Shell shell, MApplication app, EModelService modelService, 
			EPartService partService){
		boolean load = "load".equals(param);
		FileDialog dialog = new FileDialog(shell, load?SWT.OPEN:SWT.SAVE);
		dialog.setFilterExtensions (new String []{"*.scview"});
		if(!load) dialog.setFileName("SCViewer.scview");
		String res = dialog.open();
		MPart part = partService.getActivePart();
		if(res!=null && part!=null){
			Object obj = part.getObject();
			if(obj instanceof WaveformViewer){
				if(load)
					((WaveformViewer)obj).loadState(res);
				else
					((WaveformViewer)obj).saveState(res);
			}
		}
	}

}
