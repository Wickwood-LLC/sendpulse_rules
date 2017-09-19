<?php

/**
 * UI for SendPulse Variable.
 */
class SendPulseVariableRulesDataUI extends RulesDataUI implements RulesDataDirectInputFormInterface {

  public static function getDefaultMode() {
    return 'input';
  }

  public static function inputForm($name, $info, $settings, RulesPlugin $element) {
    $settings += array($name => isset($info['default value']) ? $info['default value'] : array('name' => NULL, 'value' => NULL));

    $form[$name]['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Variable name'),
      '#default_value' => $settings[$name]['name'],
      '#required' => TRUE,
    );
    $form[$name]['value'] = array(
      '#type' => 'textfield',
      '#title' => t('Variable value'),
      '#default_value' => $settings[$name]['value'],
      '#required' => TRUE,
    );
    return $form;
  }

  public static function render($value) {
    return array(
      'content' => array('#markup' => t('(name: @name, value: @value)', array('@name' => $value['name'], '@value' => $value['value']))),
      '#attributes' => array('class' => array('rules-sendpulse-variable')),
    );
  }
}

/**
 * Implements hook_rules_data_info().
 */
function sendpulse_rules_rules_data_info() {
  return array(
    'sendpulse_variable' => array(
      'label' => t('SendPulse Variable'),
      'group' => t('SendPulse'),
      'ui class' => 'SendPulseVariableRulesDataUI',
      'property info' => array(
        'name' => array(
          'type' => 'text',
          'label' => t('Variable Name'),
          'getter callback' => 'entity_property_verbatim_get',
          'setter callback' => 'entity_property_verbatim_set',
        ),
        'value' => array(
          'type' => 'text',
          'label' => t('Variable Value'),
          'getter callback' => 'entity_property_verbatim_get',
          'setter callback' => 'entity_property_verbatim_set',
        ),
      ),
    ),
  );
}

/**
 * Implements hook_rules_action_info()
 */
function sendpulse_rules_rules_action_info() {
  $items = array();
  
  if (module_exists('sendpulse_mailing_list')) {
    $items['sendpulse_add_user_to_mailing_list'] = array(
      'label' => t('Subscribe a user to mailing lists'),
      'parameter' => array(
        'account' => array('type' => 'user', 'label' => t('Subscriber')),
        'mailing_lists' => array(
          'type' => 'list<sendpulse_mailing_list>',
          'label' => t('Mailing Lists'),
          'options list' => 'sendpulse_rules_mailing_lists',
        ),
        'variables' => array(
          'type' => 'list<sendpulse_variable>',
          'label' => t('Variables'),
        ),
      ),
      'group' => t('SendPulse'),
    );
  }
  $items['sendpulse_variable_add_to_list'] = array(
    'label' => t('Add SendPulse variable to list'),
    'parameter' => array(
      'variables' => array(
        'type' => 'list<sendpulse_variable>',
        'label' => t('Variables List'),
        'save' => TRUE,
      ),
      'name' => array(
        'type' => 'text',
        'label' => t('Variable Name'),
      ),
      'value' => array(
        'type' => 'text',
        'label' => t('Variable Value'),
      ),
    ),
    'group' => t('SendPulse'),
  );
  return $items;
}

/**
 * Mailing list options.
 */
function sendpulse_rules_mailing_lists($element) {
  $mailing_lists = sendpulse_mailing_list_load_multiple();
  $options = array();
  foreach ($mailing_lists as $id => $mailing_list) {
    $options[$id] = entity_label('sendpulse_mailing_list', $mailing_list);
  }
  return $options;
}

/**
 * Rules action callback.
 */
function sendpulse_add_user_to_mailing_list($account, $mailing_lists, $variables) {
  $sendpulse = sendpulse_get_client();
  foreach ($mailing_lists as $mailing_list) {
    $data = array();
    $data['email'] = $account->mail;
    foreach ($variables as $variable) {
      $data['variables'][$variable['name']] = $variable['value'];
    }
    $result = $sendpulse->addEmails($mailing_list->sendpulse_abid, array($data));
    
    if (!isset($result->result) || $result->result != TRUE) {
      watchdog(
        'sendpulse',
        t(
          'User @username (@uid) could not be added to mailing list @ml_name (@abid).',
          array(
            '@username' => $account->name, '@uid' => $account->uid,
            '@ml_name' => $mailing_list->label, '@abid' => $mailing_list->sendpulse_abid,
          )
        )
      );
    }
  }
}

/**
 * Rules action callback.
 */
function sendpulse_variable_add_to_list($variables, $name, $value) {
  $variables[] = array ('name' => $name, 'value' => $value);
  return array('variables' => $variables);
}
