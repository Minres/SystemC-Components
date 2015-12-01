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

package com.minres.scviewer.e4.application.elements;

import java.util.List;

import javax.inject.Inject;

import org.eclipse.e4.ui.di.AboutToShow;
import org.eclipse.e4.ui.model.application.MApplication;
import org.eclipse.e4.ui.model.application.commands.MCommand;
import org.eclipse.e4.ui.model.application.commands.MCommandParameter;
import org.eclipse.e4.ui.model.application.commands.MParameter;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.model.application.ui.menu.MHandledMenuItem;
import org.eclipse.e4.ui.model.application.ui.menu.MMenuElement;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.e4.ui.workbench.modeling.EPartService;

import com.minres.scviewer.database.RelationType;
import com.minres.scviewer.e4.application.parts.WaveformViewer;

/**
 * The Class NavigateContribution. Currently not used in Application.e4xmi
 */
public class NavigateContribution {
	
	/** The part service. */
	@Inject EPartService partService;
	
	/**
	 * About to show.
	 *
	 * @param items the items
	 * @param application the application
	 * @param modelService the model service
	 */
	@AboutToShow
	public void aboutToShow(List<MMenuElement> items, MApplication application, EModelService modelService) {
//		modelService.getActivePerspective(window)
//		modelService.findElements(application,"myID",MPart.class,	EModelService.IN_ACTIVE_PERSPECTIVE);
		// MDirectMenuItem dynamicItem = MMenuFactory.INSTANCE.createDirectMenuItem();
		MPart part = partService.getActivePart();
		if(part.getObject()instanceof WaveformViewer){
			WaveformViewer waveformViewerPart = (WaveformViewer) part.getObject();
			RelationType relationTypeFilter = waveformViewerPart.getRelationTypeFilter();
			MCommand command = modelService.findElements(application, 
					"com.minres.scviewer.e4.application.command.setrelationtype", MCommand.class, null).get(0);
			MCommandParameter commandParameter = command.getParameters().get(0);
			for(RelationType relationType:waveformViewerPart.getAllRelationTypes()){
//				MDirectMenuItem dynamicItem = modelService.createModelElement(MDirectMenuItem.class);
//				
//				dynamicItem.setLabel(relationType.getName());
//				dynamicItem.setIconURI(relationTypeFilter.equals(relationType)?
//						"platform:/plugin/com.minres.scviewer.e4.application/icons/tick.png":
//							"platform:/plugin/com.minres.scviewer.e4.application/icons/empty.png");
//				dynamicItem.setContributorURI("platform:/plugin/com.minres.scviewer.e4.application");
//				dynamicItem.setContributionURI("bundleclass://com.minres.scviewer.e4.application/com.minres.scviewer.e4.application.parts.DirectMenuItem?blah=1");	
//				items.add(dynamicItem);	
				MParameter parameter=modelService.createModelElement(MParameter.class);
				parameter.setName(commandParameter.getElementId());
				parameter.setValue(relationType.getName());
				parameter.setContributorURI("platform:/plugin/com.minres.scviewer.e4.application");
				MHandledMenuItem handledMenuItem= modelService.createModelElement(MHandledMenuItem.class);
				handledMenuItem.setLabel(relationType.getName());
				if(relationTypeFilter.equals(relationType)){
					handledMenuItem.setEnabled(false);
					handledMenuItem.setIconURI("platform:/plugin/com.minres.scviewer.e4.application/icons/tick.png");
				}else
					handledMenuItem.setIconURI("platform:/plugin/com.minres.scviewer.e4.application/icons/empty.png");
				handledMenuItem.setContributorURI("platform:/plugin/com.minres.scviewer.e4.application");
				handledMenuItem.setCommand(command);
				handledMenuItem.getParameters().add(parameter);		
				items.add(handledMenuItem);	
			}
		}
	}

}