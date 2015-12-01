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
package com.minres.scviewer.e4.application;

import java.io.File;

import javax.annotation.PostConstruct;
import javax.inject.Inject;

import org.eclipse.e4.core.contexts.ContextInjectionFactory;
import org.eclipse.e4.core.contexts.IEclipseContext;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.model.application.MApplication;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.model.application.ui.basic.MPartStack;
import org.eclipse.e4.ui.workbench.UIEvents;
import org.eclipse.e4.ui.workbench.lifecycle.PostContextCreate;
import org.eclipse.e4.ui.workbench.lifecycle.PreSave;
import org.eclipse.e4.ui.workbench.lifecycle.ProcessAdditions;
import org.eclipse.e4.ui.workbench.lifecycle.ProcessRemovals;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.e4.ui.workbench.modeling.EPartService.PartState;
import org.eclipse.equinox.app.IApplicationContext;
import org.osgi.service.event.Event;
import org.osgi.service.event.EventHandler;

/**
 * This implementation contains e4 LifeCycle annotated methods.<br />
 * There is a corresponding entry in <em>plugin.xml</em> (under the
 * <em>org.eclipse.core.runtime.products' extension point</em>) that references
 * this class.
 **/
@SuppressWarnings("restriction")
public class E4LifeCycle {

	/**
	 * Post construct.
	 *
	 * @param eventBroker the event broker
	 */
	@PostConstruct
	private static void postConstruct(final IEventBroker eventBroker) {
	}

	/**
	 * Post context create.  Open a database if given on command line using the OpenViewHandler
	 *
	 * @param appContext the app context
	 * @param eventBroker the event broker
	 */
	@PostContextCreate
	void postContextCreate(IApplicationContext appContext, final IEventBroker eventBroker) {
		final String[] args = (String[])appContext.getArguments().get(IApplicationContext.APPLICATION_ARGS); 
		// react on the first view being created, at that time the UI is available
		eventBroker.subscribe(UIEvents.UILifeCycle.ACTIVATE, new EventHandler() {
			@Override
			public void handleEvent(Event event) {
				MPart part = (MPart) event.getProperty("ChangedElement");
				if(part!=null){
					IEclipseContext ctx = part.getContext();
					OpenViewHandler openViewHandler= new OpenViewHandler();
					ContextInjectionFactory.inject(openViewHandler, ctx);
					eventBroker.unsubscribe(this);
					for(String name:args){
						if(new File(name).exists())	openViewHandler.openViewForFile(name);
					}
				}
			}
		});
	}
	
	/**
	 * Pre save.
	 *
	 * @param workbenchContext the workbench context
	 */
	@PreSave
	void preSave(IEclipseContext workbenchContext) {
	}

	/**
	 * Process additions.
	 *
	 * @param workbenchContext the workbench context
	 */
	@ProcessAdditions
	void processAdditions(IEclipseContext workbenchContext) {
	}

	/**
	 * Process removals.
	 *
	 * @param workbenchContext the workbench context
	 */
	@ProcessRemovals
	void processRemovals(IEclipseContext workbenchContext) {
	}

	/**
	 * Join.
	 *
	 * @param tokens the tokens
	 * @return the string
	 */
	String join(String[] tokens){
		StringBuilder sb = new StringBuilder();
		boolean first=true;
		for(String token:tokens){
			if(!first) sb.append(",");
			sb.append(token);
			first=false;
		}
		return sb.toString();
	}
	
	/**
	 * The Class OpenViewHandler.
	 */
	private class OpenViewHandler {
		
		/** The app. */
		@Inject MApplication app;
		
		/** The model service. */
		@Inject EModelService modelService;
		
		/** The part service. */
		@Inject EPartService partService;
		
		/**
		 * Open view for file.
		 *
		 * @param name the name
		 */
		public void openViewForFile(String name){
			File file = new File(name);
			MPart part = partService.createPart("com.minres.scviewer.e4.application.partdescriptor.waveformviewer");
			part.setLabel(file.getName());
			MPartStack partStack = (MPartStack)modelService.find("org.eclipse.editorss", app);
			partStack.getChildren().add(part);
			partService.showPart(part, PartState.ACTIVATE);
			IEclipseContext ctx=part.getContext();
			ctx.modify("input", file);
			ctx.declareModifiable("input");
		}
	}
}
