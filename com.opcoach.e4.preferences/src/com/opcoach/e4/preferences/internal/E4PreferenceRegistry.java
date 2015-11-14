/*******************************************************************************
 * Copyright (c) 2014 OPCoach.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     OPCoach - initial API and implementation
 *******************************************************************************/
package com.opcoach.e4.preferences.internal;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import javax.inject.Inject;

import org.eclipse.core.runtime.IConfigurationElement;
import org.eclipse.core.runtime.IExtensionRegistry;
import org.eclipse.core.runtime.preferences.InstanceScope;
import org.eclipse.e4.core.contexts.ContextInjectionFactory;
import org.eclipse.e4.core.contexts.IEclipseContext;
import org.eclipse.e4.core.di.annotations.Creatable;
import org.eclipse.e4.core.services.contributions.IContributionFactory;
import org.eclipse.e4.core.services.log.Logger;
import org.eclipse.jface.preference.FieldEditorPreferencePage;
import org.eclipse.jface.preference.IPreferenceNode;
import org.eclipse.jface.preference.IPreferencePage;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.preference.PreferenceManager;
import org.eclipse.jface.preference.PreferenceNode;
import org.eclipse.jface.preference.PreferencePage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;

import com.opcoach.e4.preferences.IPreferenceStoreProvider;
import com.opcoach.e4.preferences.ScopedPreferenceStore;

@Creatable
public class E4PreferenceRegistry
{

	public static final String PREFS_PAGE_XP = "com.opcoach.e4.preferences.e4PreferencePages"; // $NON-NLS-1$
	public static final String PREF_STORE_PROVIDER = "com.opcoach.e4.preferences.e4PreferenceStoreProvider"; // $NON-NLS-1$
	protected static final String ELMT_PAGE = "page"; // $NON-NLS-1$
	protected static final String ATTR_ID = "id"; // $NON-NLS-1$
	protected static final String ATTR_CATEGORY = "category"; // $NON-NLS-1$
	protected static final String ATTR_CLASS = "class"; // $NON-NLS-1$
	protected static final String ATTR_NAME = "name"; // $NON-NLS-1$

	protected static final String ATTR_PLUGIN_ID = "pluginId"; // $NON-NLS-1$
	protected static final String ATTR_ID_IN_WBCONTEXT = "idInWorkbenchContext"; // $NON-NLS-1$

	@Inject
	protected Logger logger;

	@Inject
	protected IEclipseContext context;

	@Inject
	protected IExtensionRegistry registry;

	private PreferenceManager pm = null;

	// A map of (pluginId, { IPreferenceStoreProvider, or key in wbcontext }
	private Map<String, Object> psProviders;

	public PreferenceManager getPreferenceManager()
	{

		// Remember of the unbounded nodes to order parent pages.
		// Map<category, list of children> (all nodes except root nodes)
		Map<String, Collection<IPreferenceNode>> childrenNodes = new HashMap<String, Collection<IPreferenceNode>>();

		if (pm != null)
			return pm;

		pm = new PreferenceManager();
		IContributionFactory factory = context.get(IContributionFactory.class);

		for (IConfigurationElement elmt : registry.getConfigurationElementsFor(PREFS_PAGE_XP))
		{
			String bundleId = elmt.getNamespaceIdentifier();
			if (!elmt.getName().equals(ELMT_PAGE))
			{
				logger.warn("unexpected element: {0}", elmt.getName());
				continue;
			} else if (isEmpty(elmt.getAttribute(ATTR_ID)) || isEmpty(elmt.getAttribute(ATTR_NAME)))
			{
				logger.warn("missing id and/or name: {}", bundleId);
				continue;
			}
			PreferenceNode pn = null;
			if (elmt.getAttribute(ATTR_CLASS) != null)
			{
				PreferencePage page = null;
				try
				{
					String prefPageURI = getClassURI(bundleId, elmt.getAttribute(ATTR_CLASS));
					Object object = factory.create(prefPageURI, context);
					if (!(object instanceof PreferencePage))
					{
						logger.error("Expected instance of PreferencePage: {0}", elmt.getAttribute(ATTR_CLASS));
						continue;
					}
					page = (PreferencePage) object;
					setPreferenceStore(bundleId, page);

				} catch (ClassNotFoundException e)
				{
					logger.error(e);
					continue;
				}
				ContextInjectionFactory.inject(page, context);
				if ((page.getTitle() == null || page.getTitle().isEmpty()) && elmt.getAttribute(ATTR_NAME) != null)
				{
					page.setTitle(elmt.getAttribute(ATTR_NAME));
				}

				pn = new PreferenceNode(elmt.getAttribute(ATTR_ID), page);
			} else
			{
				pn = new PreferenceNode(elmt.getAttribute(ATTR_ID), new EmptyPreferencePage(elmt.getAttribute(ATTR_NAME)));
			}

			// Issue 2 : Fix bug on order (see :
			// https://github.com/opcoach/e4Preferences/issues/2)
			// Add only pages at root level and remember of child pages for
			// categories
			String category = elmt.getAttribute(ATTR_CATEGORY);
			if (isEmpty(category))
			{
				pm.addToRoot(pn);
			} else
			{
				/*
				 * IPreferenceNode parent = findNode(pm, category); if (parent
				 * == null) { // No parent found, but may be the extension has
				 * not been read yet. So remember of it unboundedNodes.put(pn,
				 * category); } else { parent.add(pn); }
				 */
				// Check if this category is already registered.
				Collection<IPreferenceNode> children = childrenNodes.get(category);
				if (children == null)
				{
					children = new ArrayList<IPreferenceNode>();
					childrenNodes.put(category, children);
				}
				children.add(pn);
			}
		}

		// Must now bind pages that has not been added in nodes (depends on the
		// preference page read order)
		// Iterate on all possible categories
		Collection<String> categoriesDone = new ArrayList<String>();

		while (!childrenNodes.isEmpty())
		{
			for (String cat : Collections.unmodifiableSet(childrenNodes.keySet()))
			{
				// Is this category already in preference manager ? If not add
				// it later...
				IPreferenceNode parent = findNode(pm, cat);
				if (parent != null)
				{
					// Can add the list of children to this parent page...
					for (IPreferenceNode pn : childrenNodes.get(cat))
					{
						parent.add(pn);
					}
					// Ok This parent page is done. Can remove it from map
					// outside of this loop
					categoriesDone.add(cat);
				}
			}

			for (String keyToRemove : categoriesDone)
				childrenNodes.remove(keyToRemove);
			categoriesDone.clear();

		}

		return pm;
	}

	private void setPreferenceStore(String bundleId, PreferencePage page)
	{
		// Affect preference store to this page if this is a
		// PreferencePage, else, must manage it internally
		// Set the issue#1 on github :
		// https://github.com/opcoach/e4Preferences/issues/1
		// And manage the extensions of IP
		initialisePreferenceStoreProviders();

		IPreferenceStore store = null;

		// Get the preference store according to policy.
		Object data = psProviders.get(bundleId);
		if (data != null)
		{
			if (data instanceof IPreferenceStore)
				store = (IPreferenceStore) data;
			else if (data instanceof IPreferenceStoreProvider)
				store = ((IPreferenceStoreProvider) data).getPreferenceStore();
			else if (data instanceof String)
				store = (IPreferenceStore) context.get((String) data);
			
		} else
		{
			// Default behavior : create a preference store for this bundle and remember of it
			store = new ScopedPreferenceStore(InstanceScope.INSTANCE, bundleId);
			psProviders.put(bundleId, store);
		}

		
		if (store != null)
			page.setPreferenceStore(store);
		else
		{
			logger.warn("Unable to set the preferenceStore for page " + page.getTitle() + " defined in bundle " + bundleId);
		}

	}

	/** Read the e4PreferenceStoreProvider extension point */
	private void initialisePreferenceStoreProviders()
	{
		if (psProviders == null)
		{
			IContributionFactory factory = context.get(IContributionFactory.class);

			psProviders = new HashMap<String, Object>();

			// Read extensions and fill the map...
			for (IConfigurationElement elmt : registry.getConfigurationElementsFor(PREF_STORE_PROVIDER))
			{
				String declaringBundle = elmt.getNamespaceIdentifier();
				String pluginId = elmt.getAttribute(ATTR_PLUGIN_ID);
				if (isEmpty(pluginId))
				{
					logger.warn("missing plugin Id in extension " + PREF_STORE_PROVIDER + " check the plugin " + declaringBundle);
					continue;
				}

				String classname = elmt.getAttribute(ATTR_CLASS);
				String objectId = elmt.getAttribute(ATTR_ID_IN_WBCONTEXT);

				if ((isEmpty(classname) && isEmpty(objectId)) || (((classname != null) && classname.length() > 0) && ((objectId != null) && objectId.length() > 0)))
				{
					logger.warn("In extension " + PREF_STORE_PROVIDER + " only one of the two attributes (pluginId or idInWorkbenchContext) must be set. Check the plugin "
							+ declaringBundle);
					continue;
				}

				// Ok can now work with data...
				Object data = objectId;
				if (classname != null)
				{
					data = factory.create(classname, context);
					if (!(data instanceof IPreferenceStoreProvider))
					{
						logger.warn("In extension " + PREF_STORE_PROVIDER + " the class must implements IPreferenceStoreProvider. Check the plugin " + declaringBundle);
						continue;
					}
				}

				psProviders.put(pluginId, data);

			}
		}
	}

	private IPreferenceNode findNode(PreferenceManager pm, String categoryId)
	{
		for (Object o : pm.getElements(PreferenceManager.POST_ORDER))
		{
			if (o instanceof IPreferenceNode && ((IPreferenceNode) o).getId().equals(categoryId))
			{
				return (IPreferenceNode) o;
			}
		}
		return null;
	}

	private String getClassURI(String definingBundleId, String spec) throws ClassNotFoundException
	{
		if (spec.startsWith("platform:"))
		{
			return spec;
		} // $NON-NLS-1$
		return "bundleclass://" + definingBundleId + '/' + spec;
	}

	private boolean isEmpty(String value)
	{
		return value == null || value.trim().isEmpty();
	}

	static class EmptyPreferencePage extends PreferencePage
	{

		public EmptyPreferencePage(String title)
		{
			setTitle(title);
			noDefaultAndApplyButton();
		}

		@Override
		protected Control createContents(Composite parent)
		{
			return new Label(parent, SWT.NONE);
		}

	}

}
