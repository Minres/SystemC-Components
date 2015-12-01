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

import java.io.File;

import org.eclipse.e4.core.contexts.IEclipseContext;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.ui.model.application.MApplication;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.model.application.ui.basic.MPartStack;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.e4.ui.workbench.modeling.EPartService.PartState;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Shell;
public class OpenHandler {

	@Execute
	public void execute(Shell shell, MApplication app, EModelService modelService, EPartService partService){
		FileDialog dialog = new FileDialog(shell, SWT.OPEN | SWT.MULTI);
//		dialog.setFilterExtensions (new String []{"vcd", "txdb", "txlog"});
		dialog.setFilterExtensions (new String []{"*.vcd;*.txdb;*.txlog"});
		dialog.open();
		String path = dialog.getFilterPath();
		for(String fileName: dialog.getFileNames()){
			File file = new File(path+File.separator+fileName);
			if(file.exists()){
//				MPart part = MBasicFactory.INSTANCE.createPart();
//				part.setLabel(fileName);
//				part.setContributionURI("bundleclass://com.minres.scviewer.e4.application/"+ WaveformViewerPart.class.getName());
				MPart part = partService .createPart("com.minres.scviewer.e4.application.partdescriptor.waveformviewer");
				part.setLabel(file.getName());
								

				MPartStack partStack = (MPartStack)modelService.find("org.eclipse.editorss", app);
				partStack.getChildren().add(part);
				partService.showPart(part, PartState.ACTIVATE);
//				Object o = part.getObject();
//				if(o instanceof WaveformViewerPart)
//					((WaveformViewerPart)o).setPartInput(file);
				IEclipseContext ctx=part.getContext();
				ctx.modify("input", file);
				ctx.declareModifiable("input");

				
			}
		}
	}
	
}
