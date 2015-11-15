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

import java.util.List;

import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.jface.viewers.IStructuredSelection;

import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.e4.application.parts.DesignBrowser;

public class AddWaveformHandler {

	public final static String PARAM_WHERE_ID="com.minres.scviewer.e4.application.command.addwaveform.where";
	public final static String PARAM_ALL_ID="com.minres.scviewer.e4.application.command.addwaveform.all";
	
	@Inject @Optional DesignBrowser designBrowser;
	
	@CanExecute
	public boolean canExecute(@Named(PARAM_WHERE_ID) String where, @Named(PARAM_ALL_ID) String all,
			EPartService partService,
			@Named(IServiceConstants.ACTIVE_SELECTION) @Optional IStructuredSelection selection) {
		if(designBrowser==null) designBrowser = getListPart( partService);
		if(designBrowser==null || designBrowser.getActiveWaveformViewerPart()==null) return false;
		Boolean before = "before".equalsIgnoreCase(where);
		if("true".equalsIgnoreCase(all)) 
			return designBrowser.getFilteredChildren().length>0 && 
					(!before || ((IStructuredSelection)designBrowser.getActiveWaveformViewerPart().getSelection()).size()>0);
		else
			return selection.size()>0 && 
					(!before || ((IStructuredSelection)designBrowser.getActiveWaveformViewerPart().getSelection()).size()>0);
	}

	@Execute
	public void execute(@Named(PARAM_WHERE_ID) String where, @Named(PARAM_ALL_ID) String all, 
			EPartService partService,
			@Named(IServiceConstants.ACTIVE_SELECTION) @Optional IStructuredSelection selection) {
		if(designBrowser==null) designBrowser = getListPart( partService);
		if(designBrowser!=null && selection.size()>0){
			List<?> sel=selection.toList();
			designBrowser.getActiveWaveformViewerPart().addStreamsToList(sel.toArray(new IWaveform<?>[]{}),
					"before".equalsIgnoreCase(where));
		}
	}

	protected DesignBrowser getListPart(EPartService partService){
		MPart part = partService.getActivePart();
		if(part.getObject() instanceof DesignBrowser)
			return (DesignBrowser) part.getObject();
		else
			return null;
	}	
}
