import { PolymerElement, html } from '@polymer/polymer/polymer-element.js';
import '@polymer/paper-button/paper-button.js';
import '@polymer/paper-styles/typography.js';
import '@polymer/paper-styles/typography.js';
import '@polymer/iron-ajax/iron-ajax.js';
import '@polymer/paper-card/paper-card.js';
import '@polymer/iron-pages/iron-pages.js';
import '@polymer/paper-input/paper-input.js';
import '@polymer/iron-flex-layout/iron-flex-layout-classes.js';
import '@polymer/paper-styles/color.js';
import '@polymer/paper-dialog/paper-dialog.js';
import '@polymer/paper-tabs/paper-tabs.js';
import '@polymer/paper-tabs/paper-tab.js';

/**
 * @customElement
 * @polymer
 */
class WebappApp extends PolymerElement {
  static get template() {
    return html`
    <style is="custom-style" include="iron-flex iron-flex-alignment">
      :host {
        transition: opacity 0.3s cubic-bezier(0.4, 0, 0.2, 1);
        background-color: lightgray;
      }

      paper-tabs {
        background-color: var(--paper-blue-900);
        color: #fff;
      }

      paper-button{
        margin: 8px 8px 8px 8px
      }
      paper-button.blue {
        background: var(--paper-blue-700);
        color: white;
      }
      paper-button.green {
        background: var(--paper-green-700);
        color: white;
      }
      paper-button.red {
        background: var(--paper-red-700);
        color: white;
      }
      paper-button.orange {
        background: var(--paper-orange-700);
        color: white;
      }
      paper-card{
        min-width: 400px;
      }

      .content {
        @apply --layout-vertical;
        padding: 16px;
        @apply --layout-center;     
      }

      .value {
        @apply --paper-font-subheading;
        font-weight: bold;
      }

      .label {
        @apply --paper-font-body2;
      }
      .subheading{
        @apply --paper-font-title;
        margin-bottom: 16px;
      }
    </style>
    <iron-ajax id="getDataAjax" auto="" url="[[_http_root]]all" handle-as="json" last-response="{{_data}}" debounce-duration="300" on-response="_onAllResponse">
    </iron-ajax>
    <iron-ajax id="getConfigAjax" auto="" url="[[_http_root]]config" handle-as="json" last-response="{{_config}}" debounce-duration="300">
    </iron-ajax>
    <iron-ajax id="saveConfigAjax" method="POST" url="[[_http_root]]save-config" content-type="application/json" debounce-duration="300" on-response="_onSaveConfigResponse" on-error="_onSaveConfigError">
    </iron-ajax>
    <iron-ajax id="rebootAjax" url="[[_http_root]]reboot" method="GET" debounce-duration="300" on-response="_onRebootResponse" on-error="_onRebootError">
    </iron-ajax>
    <iron-ajax id="doorActionAjax" url="[[_http_root]]command" handle-as="json" method="GET" params="[[_actionParams]]" debounce-duration="300" on-response="_onActionResponse"
      on-response="_onActionError">
    </iron-ajax>

    <paper-dialog id="_dialog" modal>
      <h2>[[_dialogTitle]]</h2>
      <p>[[_dialogMessage]]</p>
      <div class="buttons">
        <paper-button dialog-confirm autofocus>OK</paper-button>
      </div>
    </paper-dialog>

    <div class="content">
      <iron-pages id="_pages" selected="0">
        <paper-card heading="Garage Door">
          <div class="card-content">
              <div class="subheading">Current Information</div>
              <table fixed-column="">
              <tbody>
                <tr>
                  <td class="label">Timestamp</td>
                  <td class="value">[[_data.time_stamp]]</td>
                </tr>
                <tr>
                  <td class="label">Door Closed</td>
                  <td class="value">[[_true_false(_data.door_closed)]]</td>
                </tr>
                <tr>
                  <td class="label">Door Locked</td>
                  <td class="value">[[_true_false(_data.door_locked)]]</td>
                </tr>
                <tr>
                  <td class="label">Ultrasonic Distance</td>
                  <td class="value">[[_data.sonic_distance]] cm</td>
                </tr>
                <tr>
                  <td class="label">Temperature</td>
                  <td class="value">[[_data.temperature]]</td>
                </tr>
                <tr>
                  <td class="label">Up Time</td>
                  <td class="value">[[_data.up_time]]</td>
                </tr>
                <tr>
                  <td class="label">Boot Time</td>
                  <td class="value">[[_data.boot_time]]</td>
                </tr>
                <tr>
                  <td class="label">Free Heap</td>
                  <td class="value">[[_data.heap]]</td>
                </tr>
              </tbody>
            </table>
          </div>
          <div class="card-actions">
            <paper-button raised class="blue" disabled="[[_door_action_enabled(_data.door_locked)]]" 
              on-tap="_toggle_door">[[_door_action_title(_data.door_closed)]]</paper-button>
            <paper-button on-tap="_toggle_locked">[[_door_locked_title(_data.door_locked)]]</paper-button>
            <paper-button on-tap="_force">Force</paper-button>
            <paper-button raised class="green" on-tap="_configure">Configure</paper-button>
          </div>
        </paper-card>

        <paper-card heading="Garage Door">
          <div class="card-content">
            <div class="subheading">Configuration</div>
            <paper-tabs selected="{{_selectedConfTab}}">
              <paper-tab>Network</paper-tab>
              <paper-tab>MQTT</paper-tab>
              <!-- <paper-tab>SysLog</paper-tab> -->
            </paper-tabs>
            <iron-pages id="_ConfigPages" selected="{{_selectedConfTab}}">
              <div>
                  <paper-input always-float-label label="Hostname" value="{{_config.hostname}}"></paper-input>
                  <paper-input always-float-label label="WiFi AP Name" value="{{_config.wifi_ap}}"></paper-input>
                  <paper-input always-float-label label="WiFi Password" type="password" value="{{_config.wifi_password}}"></paper-input>
                  <paper-input always-float-label label="Temperature Update Interval (Sec)" value="{{_config.update_interval}}"></paper-input>
                  <paper-input always-float-label label="Distance Open Min CM" value="{{_config.distance_open_min}}"></paper-input>
                  <paper-input always-float-label label="Distance Open Max CM" value="{{_config.distance_open_max}}"></paper-input>
                  <!-- <paper-input always-float-label label="NTP Server" value="{{_config.ntp_server}}"></paper-input> -->
              </div>
              <div>
                <paper-input always-float-label label="Server Host" value="{{_config.mqtt_hostname}}"></paper-input>
                <paper-input always-float-label label="Server Port" value="{{_config.mqtt_port}}"></paper-input>
                <paper-input always-float-label label="Username" value="{{_config.mqtt_username}}"></paper-input>
                <paper-input always-float-label label="Password" value="{{_config.mqtt_password}}"></paper-input>
                <paper-input always-float-label label="Device ID" value="{{_config.mqtt_device_id}}"></paper-input>
                <paper-input always-float-label label="Device Name" value="{{_config.mqtt_device_name}}"></paper-input>

                <paper-input always-float-label label="Online Feed" value="{{_config.mqtt_feed_online}}"></paper-input>
                <paper-input always-float-label label="Temperature Feed" value="{{_config.mqtt_feed_temperature}}"></paper-input>
                <paper-input always-float-label label="Utrasonic Distance Feed" value="{{_config.mqtt_feed_sonic_cm}}"></paper-input>

                <paper-input always-float-label label="Door Position Command Feed" value="{{_config.mqtt_feed_position_command}}"></paper-input>
                <paper-input always-float-label label="Door Position Report Feed" value="{{_config.mqtt_feed_report_position}}"></paper-input>
                <paper-input always-float-label label="Door Lock Command Feed" value="{{_config.mqtt_feed_locked_command}}"></paper-input>
                <paper-input always-float-label label="Door Lock Report Feed" value="{{_config.mqtt_feed_report_locked}}"></paper-input>
              </div>
              <!-- <div>
                  <paper-input always-float-label label="Syslog Server" value="{{_config.syslog_server}}"></paper-input>
                  <paper-input always-float-label label="Syslog Server Port" value="{{_config.syslog_port}}"></paper-input>
                  <paper-input always-float-label label="Syslog App Name" value="{{_config.syslog_app_name}}"></paper-input>
              </div> -->

            </iron-pages>            
          </div>
          <div class="card-actions">
            <div>
              <paper-button raised class="orange" on-tap="_go_back">Back</paper-button>
              <paper-button raised class="green" on-tap="_save">Save</paper-button>
            </div>
            <div>
              <!--paper-button raised class="red" on-tap="_reboot">Reboot</paper-button -->
            </div>
          </div>
        </paper-card>

      </iron-pages>
    </div>

`;
  }

  static get is() { return 'webapp-app'; }

  static get observers() {
    return [
      '_pageChanged(_page)'
    ];
  }
  
  constructor() {
    super();
    this._data = null;
    this._actionParams = null;
    this._config = null;
    this._dialogTitle = null;
    this._dialogMessage = null;
    this._http_root = window.MyAppGlobals.rootPath;
    this._selectedConfTab = 0;
    this._page = 0;
  }

  _pageChanged(page){
    if (page == 1){ 
      this.$.getConfigAjax.generateRequest();
    }
    if(page == 0){
      this.$.getDataAjax.generateRequest();
    }
  }

  _onReadNowResponse(event) {
    this.$.getDataAjax.generateRequest();
  }

  _onReadNowError(event, error){
    this._showMessage("Read Now Error",error.error);
  }

  _configure() {
    this.$._pages.selected = 1;
  }

  _go_back() {
    this.$._pages.selected = 0;
  }

  _showMessage(title, message){
    this._dialogTitle = title;
    this._dialogMessage = message;
    this.$._dialog.open();
  }

  _onAllResponse(r) {
    console.log("response", r);
    this._intervalID = window.setTimeout(() => {
      this.$.getDataAjax.generateRequest();
    }, 5000);
  }


  _save(){
    this.$.saveConfigAjax.body = this._config;
    console.log("Sending " + this.$.saveConfigAjax.body);
    this.$.saveConfigAjax.generateRequest();
  }

  _onSaveConfigResponse(event){
    console.log("Save Response", event);
    this._showMessage("Save Config",event.detail.statusText);
  }

  _onSaveConfigError(event, error){
    this._showMessage("Save Error",error.error);
  }

  _reboot(){
    this.$.rebootAjax.generateRequest();
  }

  _onRebootResponse(event) {
    console.log("Reboot Response", event);
    this._showMessage("Reboot",event.detail.statusText);

  }

  _onRebootError(event, error){
    this._showMessage("Reboot Error",error.error);
  }


  _onActionResponse(event) {
    console.log("Action Response", event);
    this._showMessage("Action",event.detail.statusText);

  }

  _onActionError(event, error){
    this._showMessage("Action Error",error.error);
  }

  _true_false(home) {
    if (home != true && home != false) {
      return "Unknown"
    }
    return home ? "Yes" : "No";
  }

  _door_action_enabled(locked) {
    return locked;
  }

  _door_action_title(closed) {
    if (closed) {
      return "Open Door";
    } else  {
      return "Close Door"
    }
  }

  _door_locked_title(locked){
    if(locked === true){
      return 'Unlock';
    } else if (locked == false){
      return 'Lock';
    }
    return 'Noo!!!!';

  }

  _toggle_door() {
    this._actionParams = {
      "door_command": this._data.door_closed ? "OPEN" : "CLOSE"
    };
    this.$.doorActionAjax.generateRequest();
  }

  _toggle_locked() {
    this._actionParams = {
      "lock_action": this._data.door_locked ? "UNLOCK" : "LOCK"
    };
    this.$.doorActionAjax.generateRequest();
    
  }

  _force() {
    this._actionParams = {
      "door_command": "FORCE"
    };
    this.$.doorActionAjax.generateRequest();
    
  }


}


window.customElements.define(WebappApp.is, WebappApp);
