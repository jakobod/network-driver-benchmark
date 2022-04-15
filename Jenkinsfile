void setBuildStatus(String message, String state) {
  step([
      $class: "GitHubCommitStatusSetter",
      reposSource: [$class: "ManuallyEnteredRepositorySource", url: "https://github.com/jakobod/network-driver-benchmark"],
      contextSource: [$class: "ManuallyEnteredCommitContextSource", context: "ci/jenkins/build-status"],
      errorHandlers: [[$class: "ChangingBuildStatusErrorHandler", result: "UNSTABLE"]],
      statusResultSource: [ $class: "ConditionalStatusResultSource", results: [[$class: "AnyBuildResult", message: message, state: state]] ]
  ]);
}


pipeline {
  agent { dockerfile true }

  stages {
    stage('Init') {
      steps{
        setBuildStatus("Building...", "PENDING");
        sh 'rm -rf build'
      }
    }

    stage('Configure') {
      steps {
        sh './configure'
      }
    }

    stage('Build') {
      steps {
        dir('build') {
          sh 'make'
        }
      }
    }
  }

  post {
    success {
      setBuildStatus("Build succeeded!", "SUCCESS");
    }
    failure {
      setBuildStatus("Build failed!", "FAILURE");
    }
  }
}
